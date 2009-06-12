/*=============================================================================
//
//      hal_ixp425.h
//
//      IXP425 Network Processor support (register layout, etc)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter
// Date:         2002-12-08
// Purpose:      
// Description:  IXP425 Processor support.
// Usage:        #include <cyg/hal/hal_ixp425.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/
#ifndef CYGONCE_HAL_ARM_XSCALE_HAL_IXP425_H
#define CYGONCE_HAL_ARM_XSCALE_HAL_IXP425_H

#include <pkgconf/system.h>
#include <cyg/hal/hal_xscale.h>

// --------------------------------------------------------------------------
#define IXP425_PCI_WINDOW_BASE     0x48000000
#define IXP425_PCI_WINDOW_SIZE     0x04000000

#define IXP425_QMGR_BASE           0x60000000
#define IXP425_QMGR_SIZE           0x04000000

#define IXP425_MISC_CFG_BASE       0xC8000000
#define IXP425_MISC_CFG_SIZE       0x00100000

// --------------------------------------------------------------------------
// PCI Registers  (Chapter 6)
#define IXP425_PCI_CFG_BASE        0xC0000000
#define IXP425_PCI_CFG_SIZE        0x00100000
#define IXP425_PCI_NP_AD           REG32(IXP425_PCI_CFG_BASE,0x00)
#define IXP425_PCI_NP_CBE          REG32(IXP425_PCI_CFG_BASE,0x04)
#define IXP425_PCI_NP_WDATA        REG32(IXP425_PCI_CFG_BASE,0x08)
#define IXP425_PCI_NP_RDATA        REG32(IXP425_PCI_CFG_BASE,0x0C)
#define IXP425_PCI_CRP_AD_CPE      REG32(IXP425_PCI_CFG_BASE,0x10)
#define IXP425_PCI_CRP_WDATA       REG32(IXP425_PCI_CFG_BASE,0x14)
#define IXP425_PCI_CRP_RDATA       REG32(IXP425_PCI_CFG_BASE,0x18)
#define IXP425_PCI_CSR             REG32(IXP425_PCI_CFG_BASE,0x1C)
#define IXP425_PCI_ISR             REG32(IXP425_PCI_CFG_BASE,0x20)
#define IXP425_PCI_INTEN           REG32(IXP425_PCI_CFG_BASE,0x24)
#define IXP425_PCI_DMACTRL         REG32(IXP425_PCI_CFG_BASE,0x28)
#define IXP425_PCI_AHBMEMBASE      REG32(IXP425_PCI_CFG_BASE,0x2C)
#define IXP425_PCI_AHBIOBASE       REG32(IXP425_PCI_CFG_BASE,0x30)
#define IXP425_PCI_PCIMEMBASE      REG32(IXP425_PCI_CFG_BASE,0x34)
#define IXP425_PCI_AHBDOORBELL     REG32(IXP425_PCI_CFG_BASE,0x38)
#define IXP425_PCI_PCIDOORBELL     REG32(IXP425_PCI_CFG_BASE,0x3C)
#define IXP425_PCI_ATPDMA0_AHBADDR REG32(IXP425_PCI_CFG_BASE,0x40)
#define IXP425_PCI_ATPDMA0_PCIADDR REG32(IXP425_PCI_CFG_BASE,0x44)
#define IXP425_PCI_ATPDMA0_LENGTH  REG32(IXP425_PCI_CFG_BASE,0x48)
#define IXP425_PCI_ATPDMA1_AHBADDR REG32(IXP425_PCI_CFG_BASE,0x4C)
#define IXP425_PCI_ATPDMA1_PCIADDR REG32(IXP425_PCI_CFG_BASE,0x50)
#define IXP425_PCI_ATPDMA1_LENGTH  REG32(IXP425_PCI_CFG_BASE,0x54)
#define IXP425_PCI_PTADMA0_AHBADDR REG32(IXP425_PCI_CFG_BASE,0x58)
#define IXP425_PCI_PTADMA0_PCIADDR REG32(IXP425_PCI_CFG_BASE,0x5C)
#define IXP425_PCI_PTADMA0_LENGTH  REG32(IXP425_PCI_CFG_BASE,0x60)
#define IXP425_PCI_PTADMA1_AHBADDR REG32(IXP425_PCI_CFG_BASE,0x64)
#define IXP425_PCI_PTADMA1_PCIADDR REG32(IXP425_PCI_CFG_BASE,0x68)
#define IXP425_PCI_PTADMA1_LENGTH  REG32(IXP425_PCI_CFG_BASE,0x6C)

// PCI_NP_CBE bits
#define PCI_NP_CMD_IOR             0x2
#define PCI_NP_CMD_IOW             0x3
#define PCI_NP_CMD_CONFIGR         0xA
#define PCI_NP_CMD_CONFIGW         0xB
#define PCI_NP_BE_SHIFT            4

// PCI_CRP_AD_CBE bits
#define PCI_CRP_AD_CBE_READ        0
#define PCI_CRP_AD_CBE_WRITE       (1 << 16)
#define PCI_CRP_AD_CBE_BE_SHIFT    20

// PCI_CSR bits
#define PCI_CSR_HOST               (1 << 0)
#define PCI_CSR_ARBEN              (1 << 1)
#define PCI_CSR_ADS                (1 << 2)
#define PCI_CSR_PDS                (1 << 3)
#define PCI_CSR_ABE                (1 << 4)
#define PCI_CSR_DBT                (1 << 5)
#define PCI_CSR_ASE                (1 << 8)
#define PCI_CSR_IC                 (1 << 15)
#define PCI_CSR_PRST               (1 << 16)

// PCI_ISR bits
#define PCI_ISR_PSE                (1 << 0)
#define PCI_ISR_PFE                (1 << 1)
#define PCI_ISR_PPE                (1 << 2)
#define PCI_ISR_AHBE               (1 << 3)
#define PCI_ISR_APDC               (1 << 4)
#define PCI_ISR_PADC               (1 << 5)
#define PCI_ISR_ADB                (1 << 6)
#define PCI_ISR_PDB                (1 << 7)

// PCI_INTEN bits
#define PCI_INTEN_PSE              (1 << 0)
#define PCI_INTEN_PFE              (1 << 1)
#define PCI_INTEN_PPE              (1 << 2)
#define PCI_INTEN_AHBE             (1 << 3)
#define PCI_INTEN_APDC             (1 << 4)
#define PCI_INTEN_PADC             (1 << 5)
#define PCI_INTEN_ADB              (1 << 6)
#define PCI_INTEN_PDB              (1 << 7)

// --------------------------------------------------------------------------
// SDRAM Registers  (Chapter 7)
#define IXP425_SDRAM_CFG_BASE      0xCC000000
#define IXP425_SDRAM_CFG_SIZE      0x00100000
#define IXP425_SDRAM_CONFIG        REG32(IXP425_SDRAM_CFG_BASE,0x00)
#define IXP425_SDRAM_REFRESH       REG32(IXP425_SDRAM_CFG_BASE,0x04)
#define IXP425_SDRAM_IR            REG32(IXP425_SDRAM_CFG_BASE,0x08)

// SDRAM_CONFIG bits
#define SDRAM_CONFIG_2x8Mx16       0
#define SDRAM_CONFIG_4x8Mx16       1
#define SDRAM_CONFIG_2x16Mx16      2
#define SDRAM_CONFIG_4x16Mx16      3
#define SDRAM_CONFIG_2x32Mx16      4
#define SDRAM_CONFIG_4x32Mx16      5
#define SDRAM_CONFIG_CAS_3         (1 << 3)
#define SDRAM_CONFIG_RAS_3         (1 << 4)

// SDRAM_IR commands
#define SDRAM_IR_MODE_SET_CAS2     0
#define SDRAM_IR_MODE_SET_CAS3     1
#define SDRAM_IR_PRECHARGE         2
#define SDRAM_IR_NOP               3
#define SDRAM_IR_AUTO_REFRESH      4
#define SDRAM_IR_BURST_TERMINATE   5
#define SDRAM_IR_NORMAL            6

// --------------------------------------------------------------------------
// Expansion Bus Register (Chapter 9)
#define IXP425_EXP_CFG_BASE	   0xC4000000
#define IXP425_EXP_CFG_SIZE	   0x00100000
#define IXP425_EXP_CS_BASE         0x50000000
#define IXP425_EXP_CS_SIZE         0x01000000
#define IXP425_EXP_CS0             REG32(IXP425_EXP_CFG_BASE,0x00)
#define IXP425_EXP_CS1             REG32(IXP425_EXP_CFG_BASE,0x04)
#define IXP425_EXP_CS2             REG32(IXP425_EXP_CFG_BASE,0x08)
#define IXP425_EXP_CS3             REG32(IXP425_EXP_CFG_BASE,0x0C)
#define IXP425_EXP_CS4             REG32(IXP425_EXP_CFG_BASE,0x10)
#define IXP425_EXP_CS5             REG32(IXP425_EXP_CFG_BASE,0x14)
#define IXP425_EXP_CS6             REG32(IXP425_EXP_CFG_BASE,0x18)
#define IXP425_EXP_CS7             REG32(IXP425_EXP_CFG_BASE,0x1C)
#define IXP425_EXP_CNFG0           REG32(IXP425_EXP_CFG_BASE,0x20)
#define IXP425_EXP_CNFG1           REG32(IXP425_EXP_CFG_BASE,0x24)

// EXP_CSn bits
#define EXP_BYTE_EN                (1 << 0)
#define EXP_WR_EN                  (1 << 1)
#define EXP_SPLT_EN                (1 << 3)
#define EXP_MUX_EN                 (1 << 4)
#define EXP_HRDY_POL               (1 << 5)
#define EXP_BYTE_RD16              (1 << 6)
#define EXP_SZ_512                 (0 << 10)
#define EXP_SZ_1K                  (1 << 10)
#define EXP_SZ_2K                  (2 << 10)
#define EXP_SZ_4K                  (3 << 10)
#define EXP_SZ_8K                  (4 << 10)
#define EXP_SZ_16K                 (5 << 10)
#define EXP_SZ_32K                 (6 << 10)
#define EXP_SZ_64K                 (7 << 10)
#define EXP_SZ_128K                (8 << 10)
#define EXP_SZ_256K                (9 << 10)
#define EXP_SZ_512K                (10 << 10)
#define EXP_SZ_1M                  (11 << 10)
#define EXP_SZ_2M                  (12 << 10)
#define EXP_SZ_4M                  (13 << 10)
#define EXP_SZ_8M                  (14 << 10)
#define EXP_SZ_16M                 (15 << 10)
#define EXP_CYC_INTEL              (0 << 14)
#define EXP_CYC_MOTO               (1 << 14)
#define EXP_CYC_HPI                (2 << 14)

#define EXP_RECOVERY_SHIFT         16
#define EXP_HOLD_SHIFT             20
#define EXP_STROBE_SHIFT           22
#define EXP_SETUP_SHIFT            26
#define EXP_ADDR_SHIFT             28
#define EXP_CS_EN                  (1 << 31)

#define EXP_RECOVERY_T(x)   (((x) & 15) << EXP_RECOVERY_SHIFT)
#define EXP_HOLD_T(x)       (((x) & 3)  << EXP_HOLD_SHIFT)
#define EXP_STROBE_T(x)     (((x) & 15) << EXP_STROBE_SHIFT)
#define EXP_SETUP_T(x)      (((x) & 3)  << EXP_SETUP_SHIFT)
#define EXP_ADDR_T(x)       (((x) & 3)  << EXP_ADDR_SHIFT)

// EXP_CNFG0 bits
#define EXP_CNFG0_8BIT             (1 << 0)
#define EXP_CNFG0_PCI_HOST         (1 << 1)
#define EXP_CNFG0_PCI_ARB          (1 << 2)
#define EXP_CNFG0_PCI_66MHZ        (1 << 4)
#define EXP_CNFG0_MEM_MAP          (1 << 31)

// EXP_CNFG1 bits
#define EXP_CNFG1_SW_INT0          (1 << 0)
#define EXP_CNFG1_SW_INT1          (1 << 1)
#define EXP_CNFG1_BYTE_SWAP_EN     (1 << 8)


// --------------------------------------------------------------------------
// GPIO (Chapter 13)
#define IXP425_GPIO_CFG_BASE       0xC8004000
#define IXP425_GPOUTR              REG32(IXP425_GPIO_CFG_BASE,0x00)
#define IXP425_GPOER               REG32(IXP425_GPIO_CFG_BASE,0x04)
#define IXP425_GPINR               REG32(IXP425_GPIO_CFG_BASE,0x08)
#define IXP425_GPISR               REG32(IXP425_GPIO_CFG_BASE,0x0C)
#define IXP425_GPIT1R              REG32(IXP425_GPIO_CFG_BASE,0x10)
#define IXP425_GPIT2R              REG32(IXP425_GPIO_CFG_BASE,0x14)
#define IXP425_GPCLKR              REG32(IXP425_GPIO_CFG_BASE,0x18)

// GPCLKR bits
#define GPCLKR_CLK0DC_SHIFT        0
#define GPCLKR_CLK0TC_SHIFT        4
#define GPCLKR_CLK0_ENABLE         (1 << 8)
#define GPCLKR_CLK1DC_SHIFT        16
#define GPCLKR_CLK1TC_SHIFT        20
#define GPCLKR_CLK1_ENABLE         (1 << 24)

#define GPCLKR_CLK0_PCLK2   \
    ((0xf << GPCLKR_CLK0DC_SHIFT) | (0xf << GPCLKR_CLK0TC_SHIFT))

#define GPCLKR_CLK1_PCLK2   \
    ((0xf << GPCLKR_CLK1DC_SHIFT) | (0xf << GPCLKR_CLK1TC_SHIFT))

#define HAL_GPIO_OUTPUT_ENABLE(line) \
    *IXP425_GPOER &= ~(1 << (line))

#define HAL_GPIO_OUTPUT_DISABLE(line) \
    *IXP425_GPOER |= (1 << (line))

#define HAL_GPIO_OUTPUT_SET(line) \
    *IXP425_GPOUTR |= (1 << (line))

#define HAL_GPIO_OUTPUT_CLEAR(line) \
    *IXP425_GPOUTR &= ~(1 << (line))

// --------------------------------------------------------------------------
// Interrupts (Chapter 14)
#define IXP425_INTR_CFG_BASE	   0xC8003000
#define IXP425_INTR_ST	           REG32(IXP425_INTR_CFG_BASE,0x00)
#define IXP425_INTR_EN	           REG32(IXP425_INTR_CFG_BASE,0x04)
#define IXP425_INTR_SEL	           REG32(IXP425_INTR_CFG_BASE,0x08)
#define IXP425_INTR_IRQ_ST         REG32(IXP425_INTR_CFG_BASE,0x0C)
#define IXP425_INTR_FIQ_ST         REG32(IXP425_INTR_CFG_BASE,0x10)
#define IXP425_INTR_PRTY           REG32(IXP425_INTR_CFG_BASE,0x14)
#define IXP425_INTR_IRQ_ENC_ST     REG32(IXP425_INTR_CFG_BASE,0x18)
#define IXP425_INTR_FIQ_ENC_ST     REG32(IXP425_INTR_CFG_BASE,0x1C)

// --------------------------------------------------------------------------
// Timers (Chapter 15)
#define IXP425_OST_CFG_BASE        0xC8005000
#define IXP425_OST_TS              REG32(IXP425_OST_CFG_BASE,0x00)
#define IXP425_OST_TIM0            REG32(IXP425_OST_CFG_BASE,0x04)
#define IXP425_OST_TIM0_RL         REG32(IXP425_OST_CFG_BASE,0x08)
#define IXP425_OST_TIM1            REG32(IXP425_OST_CFG_BASE,0x0C)
#define IXP425_OST_TIM1_RL         REG32(IXP425_OST_CFG_BASE,0x10)
#define IXP425_OST_WDOG            REG32(IXP425_OST_CFG_BASE,0x14)
#define IXP425_OST_WDOG_ENA        REG32(IXP425_OST_CFG_BASE,0x18)
#define IXP425_OST_WDOG_KEY        REG32(IXP425_OST_CFG_BASE,0x1C)
#define IXP425_OST_STS             REG32(IXP425_OST_CFG_BASE,0x20)

// OST_STS bits
#define OST_STS_T0INT              0x01
#define OST_STS_T1INT              0x02
#define OST_STS_TSINT              0x04
#define OST_STS_WDOGINT            0x08
#define OST_STS_RESET              0x10

// OST_TIMx_RL bits
#define OST_TIM_RL_ENABLE          0x01
#define OST_TIM_RL_ONESHOT         0x02
#define OST_TIM_RL_MASK            0xFFFFFFFC


#endif // CYGONCE_HAL_ARM_XSCALE_HAL_IXP425_H
// EOF hal_ixp425.h
