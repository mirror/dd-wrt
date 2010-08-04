/*
 *  arch/arm/mach-cns3xxx/include/mach/platform.h
 *
 *  Copyright (c) 2008 Cavium Networks 
 *  Copyright (c) ARM Limited 2003.  All rights reserved.
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
 */

#ifndef __ASM_ARCH_PLATFORM_H
#define __ASM_ARCH_PLATFORM_H

#ifndef __ASSEMBLY__

#include <linux/io.h>

/* 
 *  SDRAM
 */
#define CNS3XXX_SDRAM_BASE           0x00000000

/* ------------------------------------------------------------------------
 *  Cavium Networks Registers
 * ------------------------------------------------------------------------
 * 
 */
#define CNS3XXX_SYS_ID_OFFSET               0x00
#define CNS3XXX_SYS_SW_OFFSET               0x04
#define CNS3XXX_SYS_LED_OFFSET              0x08
#define CNS3XXX_SYS_OSC0_OFFSET             0x0C

#define CNS3XXX_SYS_OSC1_OFFSET             0x10
#define CNS3XXX_SYS_OSC2_OFFSET             0x14
#define CNS3XXX_SYS_OSC3_OFFSET             0x18
#define CNS3XXX_SYS_OSC4_OFFSET             0x1C	/* OSC1 for Cavium Networks/AB */

#define CNS3XXX_SYS_LOCK_OFFSET             0x20
#define CNS3XXX_SYS_100HZ_OFFSET            0x24
#define CNS3XXX_SYS_CFGDATA1_OFFSET         0x28
#define CNS3XXX_SYS_CFGDATA2_OFFSET         0x2C
#define CNS3XXX_SYS_FLAGS_OFFSET            0x30
#define CNS3XXX_SYS_FLAGSSET_OFFSET         0x30
#define CNS3XXX_SYS_FLAGSCLR_OFFSET         0x34
#define CNS3XXX_SYS_NVFLAGS_OFFSET          0x38
#define CNS3XXX_SYS_NVFLAGSSET_OFFSET       0x38
#define CNS3XXX_SYS_NVFLAGSCLR_OFFSET       0x3C
#define CNS3XXX_SYS_RESETCTL_OFFSET         0x40
#define CNS3XXX_SYS_PCICTL_OFFSET           0x44
#define CNS3XXX_SYS_MCI_OFFSET              0x48
#define CNS3XXX_SYS_FLASH_OFFSET            0x4C
#define CNS3XXX_SYS_CLCD_OFFSET             0x50
#define CNS3XXX_SYS_CLCDSER_OFFSET          0x54
#define CNS3XXX_SYS_BOOTCS_OFFSET           0x58
#define CNS3XXX_SYS_24MHz_OFFSET            0x5C
#define CNS3XXX_SYS_MISC_OFFSET             0x60
#define CNS3XXX_SYS_IOSEL_OFFSET            0x70
#define CNS3XXX_SYS_PROCID_OFFSET           0x84
#define CNS3XXX_SYS_TEST_OSC0_OFFSET        0xC0
#define CNS3XXX_SYS_TEST_OSC1_OFFSET        0xC4
#define CNS3XXX_SYS_TEST_OSC2_OFFSET        0xC8
#define CNS3XXX_SYS_TEST_OSC3_OFFSET        0xCC
#define CNS3XXX_SYS_TEST_OSC4_OFFSET        0xD0

#define CNS3XXX_SYS_BASE                    0x10000000
#define CNS3XXX_SYS_ID                      (CNS3XXX_SYS_BASE + CNS3XXX_SYS_ID_OFFSET)
#define CNS3XXX_SYS_SW                      (CNS3XXX_SYS_BASE + CNS3XXX_SYS_SW_OFFSET)
#define CNS3XXX_SYS_LED                     (CNS3XXX_SYS_BASE + CNS3XXX_SYS_LED_OFFSET)
#define CNS3XXX_SYS_OSC0                    (CNS3XXX_SYS_BASE + CNS3XXX_SYS_OSC0_OFFSET)
#define CNS3XXX_SYS_OSC1                    (CNS3XXX_SYS_BASE + CNS3XXX_SYS_OSC1_OFFSET)

#define CNS3XXX_SYS_LOCK                    (CNS3XXX_SYS_BASE + CNS3XXX_SYS_LOCK_OFFSET)
#define CNS3XXX_SYS_100HZ                   (CNS3XXX_SYS_BASE + CNS3XXX_SYS_100HZ_OFFSET)
#define CNS3XXX_SYS_CFGDATA1                (CNS3XXX_SYS_BASE + CNS3XXX_SYS_CFGDATA1_OFFSET)
#define CNS3XXX_SYS_CFGDATA2                (CNS3XXX_SYS_BASE + CNS3XXX_SYS_CFGDATA2_OFFSET)
#define CNS3XXX_SYS_FLAGS                   (CNS3XXX_SYS_BASE + CNS3XXX_SYS_FLAGS_OFFSET)
#define CNS3XXX_SYS_FLAGSSET                (CNS3XXX_SYS_BASE + CNS3XXX_SYS_FLAGSSET_OFFSET)
#define CNS3XXX_SYS_FLAGSCLR                (CNS3XXX_SYS_BASE + CNS3XXX_SYS_FLAGSCLR_OFFSET)
#define CNS3XXX_SYS_NVFLAGS                 (CNS3XXX_SYS_BASE + CNS3XXX_SYS_NVFLAGS_OFFSET)
#define CNS3XXX_SYS_NVFLAGSSET              (CNS3XXX_SYS_BASE + CNS3XXX_SYS_NVFLAGSSET_OFFSET)
#define CNS3XXX_SYS_NVFLAGSCLR              (CNS3XXX_SYS_BASE + CNS3XXX_SYS_NVFLAGSCLR_OFFSET)
#define CNS3XXX_SYS_RESETCTL                (CNS3XXX_SYS_BASE + CNS3XXX_SYS_RESETCTL_OFFSET)
#define CNS3XXX_SYS_PCICTL                  (CNS3XXX_SYS_BASE + CNS3XXX_SYS_PCICTL_OFFSET)
#define CNS3XXX_SYS_MCI                     (CNS3XXX_SYS_BASE + CNS3XXX_SYS_MCI_OFFSET)
#define CNS3XXX_SYS_FLASH                   (CNS3XXX_SYS_BASE + CNS3XXX_SYS_FLASH_OFFSET)
#define CNS3XXX_SYS_CLCD                    (CNS3XXX_SYS_BASE + CNS3XXX_SYS_CLCD_OFFSET)
#define CNS3XXX_SYS_CLCDSER                 (CNS3XXX_SYS_BASE + CNS3XXX_SYS_CLCDSER_OFFSET)
#define CNS3XXX_SYS_BOOTCS                  (CNS3XXX_SYS_BASE + CNS3XXX_SYS_BOOTCS_OFFSET)
#define CNS3XXX_SYS_24MHz                   (CNS3XXX_SYS_BASE + CNS3XXX_SYS_24MHz_OFFSET)
#define CNS3XXX_SYS_MISC                    (CNS3XXX_SYS_BASE + CNS3XXX_SYS_MISC_OFFSET)
#define CNS3XXX_SYS_IOSEL                   (CNS3XXX_SYS_BASE + CNS3XXX_SYS_IOSEL_OFFSET)
#define CNS3XXX_SYS_PROCID                  (CNS3XXX_SYS_BASE + CNS3XXX_SYS_PROCID_OFFSET)
#define CNS3XXX_SYS_TEST_OSC0               (CNS3XXX_SYS_BASE + CNS3XXX_SYS_TEST_OSC0_OFFSET)
#define CNS3XXX_SYS_TEST_OSC1               (CNS3XXX_SYS_BASE + CNS3XXX_SYS_TEST_OSC1_OFFSET)
#define CNS3XXX_SYS_TEST_OSC2               (CNS3XXX_SYS_BASE + CNS3XXX_SYS_TEST_OSC2_OFFSET)
#define CNS3XXX_SYS_TEST_OSC3               (CNS3XXX_SYS_BASE + CNS3XXX_SYS_TEST_OSC3_OFFSET)
#define CNS3XXX_SYS_TEST_OSC4               (CNS3XXX_SYS_BASE + CNS3XXX_SYS_TEST_OSC4_OFFSET)

/* 
 * Values for CNS3XXX_SYS_RESET_CTRL
 */
#define CNS3XXX_SYS_CTRL_RESET_CONFIGCLR    0x01
#define CNS3XXX_SYS_CTRL_RESET_CONFIGINIT   0x02
#define CNS3XXX_SYS_CTRL_RESET_DLLRESET     0x03
#define CNS3XXX_SYS_CTRL_RESET_PLLRESET     0x04
#define CNS3XXX_SYS_CTRL_RESET_POR          0x05
#define CNS3XXX_SYS_CTRL_RESET_DoC          0x06

#define CNS3XXX_SYS_CTRL_LED         (1 << 0)


/* ------------------------------------------------------------------------
 *  Cavium Networks control registers
 * ------------------------------------------------------------------------
 */

/* 
 * CNS3XXX_IDFIELD
 *
 * 31:24 = manufacturer (0x41 = ARM)
 * 23:16 = architecture (0x08 = AHB system bus, ASB processor bus)
 * 15:12 = FPGA (0x3 = XVC600 or XVC600E)
 * 11:4  = build value
 * 3:0   = revision number (0x1 = rev B (AHB))
 */

/*
 * CNS3XXX_SYS_LOCK
 *     control access to SYS_OSCx, SYS_CFGDATAx, SYS_RESETCTL, 
 *     SYS_CLD, SYS_BOOTCS
 */
#define CNS3XXX_SYS_LOCK_LOCKED    (1 << 16)
#define CNS3XXX_SYS_LOCKVAL_MASK	0xFFFF		/* write 0xA05F to enable write access */

/*
 * CNS3XXX_SYS_FLASH
 */
#define CNS3XXX_FLASHPROG_FLVPPEN	(1 << 0)	/* Enable writing to flash */

/*
 * CNS3XXX_INTREG
 *     - used to acknowledge and control MMCI and UART interrupts 
 */
#define CNS3XXX_INTREG_WPROT        0x00    /* MMC protection status (no interrupt generated) */
#define CNS3XXX_INTREG_RI0          0x01    /* Ring indicator UART0 is asserted,              */
#define CNS3XXX_INTREG_CARDIN       0x08    /* MMCI card in detect                            */
                                                /* write 1 to acknowledge and clear               */
#define CNS3XXX_INTREG_RI1          0x02    /* Ring indicator UART1 is asserted,              */
#define CNS3XXX_INTREG_CARDINSERT   0x03    /* Signal insertion of MMC card                   */

/*
 * Cavium Networks common peripheral addresses
 */
#define CNS3XXX_SCTL_BASE            0x10001000	/* System controller */

/* PCI space */
#define CNS3XXX_PCI_BASE             0x41000000	/* PCI Interface */
#define CNS3XXX_PCI_CFG_BASE         0x42000000
#define CNS3XXX_PCI_MEM_BASE0        0x44000000
#define CNS3XXX_PCI_MEM_BASE1        0x50000000
#define CNS3XXX_PCI_MEM_BASE2        0x60000000
/* Sizes of above maps */
#define CNS3XXX_PCI_BASE_SIZE	     0x01000000
#define CNS3XXX_PCI_CFG_BASE_SIZE    0x02000000
#define CNS3XXX_PCI_MEM_BASE0_SIZE   0x0c000000	/* 32Mb */
#define CNS3XXX_PCI_MEM_BASE1_SIZE   0x10000000	/* 256Mb */
#define CNS3XXX_PCI_MEM_BASE2_SIZE   0x10000000	/* 256Mb */

#define CNS3XXX_SDRAM67_BASE         0x70000000	/* SDRAM banks 6 and 7 */
#define CNS3XXX_LT_BASE              0x80000000	/* Logic Tile expansion */

/* 
 *  LED settings, bits [7:0]
 */
#define CNS3XXX_SYS_LED0             (1 << 0)
#define CNS3XXX_SYS_LED1             (1 << 1)
#define CNS3XXX_SYS_LED2             (1 << 2)
#define CNS3XXX_SYS_LED3             (1 << 3)
#define CNS3XXX_SYS_LED4             (1 << 4)
#define CNS3XXX_SYS_LED5             (1 << 5)
#define CNS3XXX_SYS_LED6             (1 << 6)
#define CNS3XXX_SYS_LED7             (1 << 7)

#define ALL_LEDS                  0xFF

#define LED_BANK                  CNS3XXX_SYS_LED

/* 
 * Control registers
 */
#define CNS3XXX_IDFIELD_OFFSET		0x0	/* Cavium Networks build information */
#define CNS3XXX_FLASHPROG_OFFSET	0x4	/* Flash devices */
#define CNS3XXX_INTREG_OFFSET		0x8	/* Interrupt control */
#define CNS3XXX_DECODE_OFFSET		0xC	/* Fitted logic modules */

/*
 * System controller bit assignment
 */
#define CNS3XXX_REFCLK	0
#define CNS3XXX_TIMCLK	1

#define CNS3XXX_TIMER1_EnSel	15
#define CNS3XXX_TIMER2_EnSel	17
#define CNS3XXX_TIMER3_EnSel	19
#define CNS3XXX_TIMER4_EnSel	21


#define MAX_TIMER                       2
#define MAX_PERIOD                      699050
#define TICKS_PER_uSEC                  1

/* 
 *  These are useconds NOT ticks.  
 * 
 */
#define mSEC_1                          1000
#define mSEC_5                          (mSEC_1 * 5)
#define mSEC_10                         (mSEC_1 * 10)
#define mSEC_25                         (mSEC_1 * 25)
#define SEC_1                           (mSEC_1 * 1000)

#define CNS3XXX_CSR_BASE             0x10000000
#define CNS3XXX_CSR_SIZE             0x10000000

/* Platform Level Setup Functions */

extern void cns3xxx_sys_init(void);
extern int cns3xxx_pcie_init(u8 ports);

/* Information about built-in Ethernet MAC interfaces */
struct eth_plat_info {
	u8 ports;	/* Bitmap of enabled Ports */
	u8 eth0_hwaddr[6];
	u8 eth1_hwaddr[6];
	u8 eth2_hwaddr[6];
	u8 cpu_hwaddr[6];
};

// Config 1 Bitmap
#define ETH0_LOAD						BIT(0)
#define ETH1_LOAD						BIT(1)
#define ETH2_LOAD						BIT(2)
#define SATA0_LOAD					BIT(3)
#define SATA1_LOAD					BIT(4)
#define PCM_LOAD						BIT(5)
#define I2S_LOAD						BIT(6)
#define SPI0_LOAD						BIT(7)
#define SPI1_LOAD						BIT(8)
#define PCIe0_LOAD					BIT(9)
#define PCIe1_LOAD					BIT(10)
#define USB0_LOAD						BIT(11)
#define USB1_LOAD						BIT(12)
#define USB1_ROUTE					BIT(13)
#define SD_LOAD							BIT(14)
#define UART0_LOAD					BIT(15)
#define UART1_LOAD					BIT(16)
#define UART2_LOAD					BIT(17)
#define mPCI0_LOAD					BIT(18)
#define mPCI1_LOAD					BIT(19)
#define mPCI2_LOAD					BIT(20)
#define mPCI3_LOAD					BIT(21)
#define FP_BUT_LOAD					BIT(22)
#define FP_BUT_HEADER_LOAD	BIT(23)
#define FP_LED_LOAD					BIT(24)
#define FP_LED_HEADER_LOAD	BIT(25)
#define FP_TAMPER_LOAD			BIT(26)
#define HEADER_33v_LOAD			BIT(27)
#define SATA_POWER_LOAD			BIT(28)
#define FP_POWER_LOAD				BIT(29)
#define GPIO_HEADER_LOAD		BIT(30)
#define GSP_BAT_LOAD				BIT(31)

// Config 2 Bitmap
#define FAN_LOAD						BIT(0)
#define SPI_FLASH_LOAD			BIT(1)
#define NOR_FLASH_LOAD			BIT(2)
#define GPS_LOAD						BIT(3)
#define SUPPLY_5v_LOAD			BIT(6)
#define SUPPLY_33v_LOAD			BIT(7)


#endif	/* __ASM_ARCH_PLATFORM_H */
#endif
