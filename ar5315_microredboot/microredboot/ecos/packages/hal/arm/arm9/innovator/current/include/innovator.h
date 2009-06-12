#ifndef CYGONCE_INNOVATOR_H
#define CYGONCE_INNOVATOR_H

//=============================================================================
//
//      innovator.h
//
//      Platform specific support (register layout, etc)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Author(s):    Patrick Doyle <wpd@delcomsys.com>
// Contributors: Patrick Doyle <wpd@delcomsys.com>
// Date:         2002-12-01
// Purpose:      Innovator platform specific support definitions
// Description: 
// Usage:        #include <cyg/hal/innovator.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal_arm_arm9_innovator.h>

// Memory mapping
#define FLASH_CS0_BASE     0x00000000
#define FLASH_CS1_BASE     0x04000000
#define FLASH_CS2_BASE     0x08000000
#define FLASH_CS3_BASE     0x0C000000
#define SDRAM_BASE         0x10000000
#define INTERNAL_SRAM_BASE 0x20000000

// Most of this should be moved to a separate omap.h or omap_1510.h file
// CLKM Registers
#define CLKM_BASE         0xFFFECE00
#define _CLKM_ARM_CKCTL   0x00
#define _CLKM_ARM_IDLECT1 0x04
#define _CLKM_ARM_IDLECT2 0x08
#define _CLKM_ARM_EWUPCT  0x0C
#define _CLKM_ARM_RSTCT1  0x10
#define _CLKM_ARM_RSTCT2  0x14
#define _CLKM_ARM_SYSST   0x18

#define CLKM_ARM_CKCTL   (volatile short *)(CLKM_BASE + _CLKM_ARM_CKCTL)
#define CLKM_ARM_IDLECT1 (volatile short *)(CLKM_BASE + _CLKM_ARM_IDLECT1)
#define CLKM_ARM_IDLECT2 (volatile short *)(CLKM_BASE + _CLKM_ARM_IDLECT2)
#define CLKM_ARM_EWUPCT  (volatile short *)(CLKM_BASE + _CLKM_ARM_EWUPCT)
#define CLKM_ARM_RSTCT1  (volatile short *)(CLKM_BASE + _CLKM_ARM_RSTCT1)
#define CLKM_ARM_RSTCT2  (volatile short *)(CLKM_BASE + _CLKM_ARM_RSTCT2)
#define CLKM_ARM_SYSST   (volatile short *)(CLKM_BASE + _CLKM_ARM_SYSST)

// Watchdog Registers
#define WATCHDOG_BASE 0xFFFEC800
#define _WD_CNTL_TIMER 0x00
#define _WD_LOAD_TIM   0x04
#define _WD_READ_TIM   0x04
#define _WD_TIMER_MODE 0x08

#define WATCHDOG_CNTL_TIMER (volatile short *)(WATCHDOG_BASE + _WD_CNTL_TIMER)
#define WATCHDOG_LOAD_TIM   (volatile short *)(WATCHDOG_BASE + _WD_LOAD_TIM)
#define WATCHDOG_READ_TIM   (volatile short *)(WATCHDOG_BASE + _WD_READ_TIM)
#define WATCHDOG_TIMER_MODE (volatile short *)(WATCHDOG_BASE + _WD_TIMER_MODE)

/* Nothing below this line has been scrubbed yet */
/* DPLL Registers */
#define DPLL1_BASE    0xFFFECF00
#define DPLL2_BASE    0xFFFED000
#define DPLL3_BASE    0xFFFED100

#define _DPLL_CTL_REG 0x00

#define DPLL1_CTL_REG (volatile short *)(DPLL1_BASE + _DPLL_CTL_REG)
#define DPLL2_CTL_REG (volatile short *)(DPLL2_BASE + _DPLL_CTL_REG)
#define DPLL3_CTL_REG (volatile short *)(DPLL3_BASE + _DPLL_CTL_REG)

#define FPGA_BASE     0x08000000    /* CS2_BASE */
#define CONFIG_BASE   0xFFFE1000
#define TC_BASE       0xFFFECC00

/* FPGA Registers */
#define FPGA_PWR_CTRL_REG      (*(volatile char     *)(FPGA_BASE     + 0x05))

/* Configuration Registers */
#define CONFIG_FUNC_MUX_CTRL_0 (*(volatile unsigned *)(CONFIG_BASE   + 0x00))
#define CONFIG_FUNC_MUX_CTRL_1 (*(volatile unsigned *)(CONFIG_BASE   + 0x04))
#define CONFIG_FUNC_MUX_CTRL_2 (*(volatile unsigned *)(CONFIG_BASE   + 0x08))

/* Traffic Controller Registers */
#define TC_IMIF_PRIO           (*(volatile unsigned *)(TC_BASE       + 0x00))
#define TC_EMIFS_PRIO_REG      (*(volatile unsigned *)(TC_BASE       + 0x04))
#define TC_EMIFF_PRIO_REG      (*(volatile unsigned *)(TC_BASE       + 0x08))
#define TC_EMIFS_CONFIG_REG    (*(volatile unsigned *)(TC_BASE       + 0x0C))
#define TC_EMIFS_CS0_CONFIG    (*(volatile unsigned *)(TC_BASE       + 0x10))
#define TC_EMIFS_CS1_CONFIG    (*(volatile unsigned *)(TC_BASE       + 0x14))
#define TC_EMIFS_CS2_CONFIG    (*(volatile unsigned *)(TC_BASE       + 0x18))
#define TC_EMIFS_CS3_CONFIG    (*(volatile unsigned *)(TC_BASE       + 0x1C))
#define TC_EMIFF_SDRAM_CONFIG  (*(volatile unsigned *)(TC_BASE       + 0x20))
#define TC_EMIFF_MRS           (*(volatile unsigned *)(TC_BASE       + 0x24))
#define TC_TIMEOUT1            (*(volatile unsigned *)(TC_BASE       + 0x28))
#define TC_TIMEOUT2            (*(volatile unsigned *)(TC_BASE       + 0x2C))
#define TC_TIMEOUT3            (*(volatile unsigned *)(TC_BASE       + 0x30))
#define TC_ENDIANISM           (*(volatile unsigned *)(TC_BASE       + 0x34))

/* Believe it or not, these actually make the code more readable in a few
 * places.
 */
#ifndef BIT_00
#define BIT_00 0x00000001
#define BIT_01 0x00000002
#define BIT_02 0x00000004
#define BIT_03 0x00000008
#define BIT_04 0x00000010
#define BIT_05 0x00000020
#define BIT_06 0x00000040
#define BIT_07 0x00000080
#define BIT_08 0x00000100
#define BIT_09 0x00000200
#define BIT_10 0x00000400
#define BIT_11 0x00000800
#define BIT_12 0x00001000
#define BIT_13 0x00002000
#define BIT_14 0x00004000
#define BIT_15 0x00008000
#define BIT_16 0x00010000
#define BIT_17 0x00020000
#define BIT_18 0x00040000
#define BIT_19 0x00080000
#define BIT_20 0x00100000
#define BIT_21 0x00200000
#define BIT_22 0x00400000
#define BIT_23 0x00800000
#define BIT_24 0x01000000
#define BIT_25 0x02000000
#define BIT_26 0x04000000
#define BIT_27 0x08000000
#define BIT_28 0x10000000
#define BIT_29 0x20000000
#define BIT_30 0x40000000
#define BIT_31 0x80000000
#endif

// We are using this
#define INNOVATOR_FLASH_VIRT_BASE 0x10000000
//  #define INNOVATOR_FLASH_PHYS_BASE 0x00000000
#define INNOVATOR_SDRAM_PHYS_BASE 0x10000000

#if 0
#define INNOVATOR_BASE         INNOVATOR_REGS_PHYS_BASE

//-----------------------------------------------------------------------------
// Boot control
// Note: this register is actually write-bit-to-clear-it
#define INNOVATOR_BOOT_CR      (INNOVATOR_BASE + 0x0000)

#define INNOVATOR_BOOT_CR_BM   0x00000001
#define INNOVATOR_BOOT_CR_HM   0x00000002
#define INNOVATOR_BOOT_CR_RE   0x00000004

//-----------------------------------------------------------------------------
// DPSRAM config
#define INNOVATOR_DPSRAM_BASE  (INNOVATOR_BASE + 0x0030)
#define _DPSRAM0_SR            0x0000
#define _DPSRAM0_LCR           0x0004
#define _DPSRAM1_SR            0x0008
#define _DPSRAM1_LCR           0x000c

#define _DPSRAM0_LCR_INIT      0x00000000
#define _DPSRAM1_LCR_INIT      0x00000000

//-----------------------------------------------------------------------------
// IO controller
#define INNOVATOR_IOCR_BASE    (INNOVATOR_BASE + 0x0040)
#define _IOCR_SDRAM            0x0000
#define _IOCR_EBI              0x0004
#define _IOCR_UART             0x0008
#define _IOCR_TRACE            0x000c

#define _IOCR_OC_PCI           0x00000008
#define _IOCR_OC_FAST          0x00000004
#define _IOCR_OC_SLOW          0x00000000
#define _IOCR_IO_STRIPE        0x00000002
#define _IOCR_LOCK             0x00000001

#define INNOVATOR_IOCR_SDRAM_INIT (_IOCR_OC_FAST | _IOCR_IO_STRIPE | _IOCR_LOCK)
#define INNOVATOR_IOCR_EBI_INIT   (_IOCR_OC_SLOW | _IOCR_IO_STRIPE | _IOCR_LOCK)
#define INNOVATOR_IOCR_UART_INIT  (_IOCR_OC_SLOW | _IOCR_IO_STRIPE | _IOCR_LOCK)


//-----------------------------------------------------------------------------
// Memory mapping
#define INNOVATOR_MMAP_BASE    (INNOVATOR_BASE + 0x0080)
#define _MMAP_REGISTERS        0x0000
#define _MMAP_SRAM0            0x0010
#define _MMAP_SRAM1            0x0014
#define _MMAP_DPSRAM0          0x0020
#define _MMAP_DPSRAM1          0x0024
#define _MMAP_SDRAM0           0x0030
#define _MMAP_SDRAM1           0x0034
#define _MMAP_EBI0             0x0040
#define _MMAP_EBI1             0x0044
#define _MMAP_EBI2             0x0048
#define _MMAP_EBI3             0x004c
#define _MMAP_PLD0             0x0050
#define _MMAP_PLD1             0x0054
#define _MMAP_PLD2             0x0058
#define _MMAP_PLD3             0x005c

#define _MMAP_SIZE_16K   (13<<7)
#define _MMAP_SIZE_64K   (15<<7)
#define _MMAP_SIZE_128K  (16<<7)
#define _MMAP_SIZE_1M    (19<<7)
#define _MMAP_SIZE_4M    (21<<7)
#define _MMAP_SIZE_16M   (23<<7)
#define _MMAP_SIZE_32M   (24<<7)
#define _MMAP_SIZE_64M   (25<<7)

#define _MMAP_PREFETCH         0x00000000
#define _MMAP_NOPREFETCH       0x00000002

#define _MMAP_ENABLE           0x00000001
#define _MMAP_DISABLE          0x00000000

#define _MMAP_REGISTERS_INIT   (INNOVATOR_REGS_PHYS_BASE  + 0x00000000 | _MMAP_SIZE_16K  | _MMAP_NOPREFETCH | _MMAP_ENABLE)
#define _MMAP_SRAM0_INIT       (INNOVATOR_SRAM_PHYS_BASE  + 0x00000000 | _MMAP_SIZE_128K | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_SRAM1_INIT       (INNOVATOR_SRAM_PHYS_BASE  + 0x00020000 | _MMAP_SIZE_128K | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_DPSRAM0_INIT     (INNOVATOR_SRAM_PHYS_BASE  + 0x00040000 | _MMAP_SIZE_64K  | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_DPSRAM1_INIT     (INNOVATOR_SRAM_PHYS_BASE  + 0x00050000 | _MMAP_SIZE_64K  | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_SDRAM0_INIT      (INNOVATOR_SDRAM_PHYS_BASE + 0x00000000 | _MMAP_SIZE_64M  | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_SDRAM1_INIT      (INNOVATOR_SDRAM_PHYS_BASE + 0x04000000 | _MMAP_SIZE_64M  | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_EBI0_INIT        (INNOVATOR_FLASH_PHYS_BASE + 0x00000000 | _MMAP_SIZE_4M   | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_EBI1_INIT        (INNOVATOR_FLASH_PHYS_BASE + 0x00400000 | _MMAP_SIZE_4M   | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_EBI2_INIT        (INNOVATOR_FLASH_PHYS_BASE + 0x00800000 | _MMAP_SIZE_4M   | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_EBI3_INIT        (INNOVATOR_FLASH_PHYS_BASE + 0x00c00000 | _MMAP_SIZE_4M   | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_PLD0_INIT        (INNOVATOR_PLD_PHYS_BASE   + 0x00000000 | _MMAP_SIZE_16K | _MMAP_NOPREFETCH | _MMAP_ENABLE)
//#define _MMAP_PLD1_INIT        (INNOVATOR_PLD_PHYS_BASE   + 0x00004000 | _MMAP_SIZE_16K | _MMAP_NOPREFETCH | _MMAP_ENABLE)
#define _MMAP_PLD1_INIT        (0x0f000000 | _MMAP_SIZE_16K | _MMAP_NOPREFETCH | _MMAP_ENABLE)
#define _MMAP_PLD2_INIT        (INNOVATOR_PLD_PHYS_BASE   + 0x00008000 | _MMAP_SIZE_16K | _MMAP_NOPREFETCH | _MMAP_ENABLE)
#define _MMAP_PLD3_INIT        (INNOVATOR_PLD_PHYS_BASE   + 0x0000c000 | _MMAP_SIZE_16K | _MMAP_NOPREFETCH | _MMAP_ENABLE)





//-----------------------------------------------------------------------------
// Timers
#define INNOVATOR_TIMER0_CR              (INNOVATOR_BASE+0x0200)
#define INNOVATOR_TIMER0_PRE             (INNOVATOR_BASE+0x0210)
#define INNOVATOR_TIMER0_LIMIT           (INNOVATOR_BASE+0x0220)
#define INNOVATOR_TIMER0_READ            (INNOVATOR_BASE+0x0230)

#define INNOVATOR_TIMER1_CR              (INNOVATOR_BASE+0x0240)
#define INNOVATOR_TIMER1_PRE             (INNOVATOR_BASE+0x0250)
#define INNOVATOR_TIMER1_LIMIT           (INNOVATOR_BASE+0x0260)
#define INNOVATOR_TIMER1_READ            (INNOVATOR_BASE+0x0270)

#define INNOVATOR_TIMER_CR_MODE_HEARBEAT 0x00000000
#define INNOVATOR_TIMER_CR_MODE_ONE_SHOT 0x00000001
#define INNOVATOR_TIMER_CR_IE            0x00000004
#define INNOVATOR_TIMER_CR_CI            0x00000008
#define INNOVATOR_TIMER_CR_S             0x00000010

//-----------------------------------------------------------------------------
// Serial
#define INNOVATOR_UART0_BASE            (INNOVATOR_BASE+0x0280)
#define _UART_RSR              0x0000
#define _UART_RDS              0x0004
#define _UART_RD               0x0008
#define _UART_TSR              0x000c
#define _UART_TD               0x0010
#define _UART_FCR              0x0014
#define _UART_IES              0x0018
#define _UART_IEC              0x001c
#define _UART_ISR              0x0020
#define _UART_IID              0x0024
#define _UART_MC               0x0028
#define _UART_MCR              0x002c
#define _UART_MSR              0x0030
#define _UART_DIV_LO           0x0034
#define _UART_DIV_HI           0x0038

#define _UART_RSR_RX_LEVEL     0x0000001f

#define _UART_TSR_TXI          0x00000080

#define _UART_FCR_TC           0x00000001
#define _UART_FCR_RC           0x00000002
#define _UART_FCR_TX_THR_15    0x0000001c
#define _UART_FCR_RX_THR_1     0x00000000

#define _UART_INTS_RE          0x00000001
#define _UART_INTS_RI          _UART_INTS_RE
#define _UART_INTS_TE          0x00000002
#define _UART_INTS_TI          _UART_INTS_TE
#define _UART_INTS_TIE         0x00000004
#define _UART_INTS_TII         _UART_INTS_TIE
#define _UART_INTS_ME          0x00000008
#define _UART_INTS_MI          _UART_INTS_ME


#define _UART_MC_8BIT          0x00000003
#define _UART_MC_1STOP         0x00000000
#define _UART_MC_PARITY_NONE   0x00000000

//-----------------------------------------------------------------------------
// Clock controller
#define INNOVATOR_CLK_BASE     (INNOVATOR_BASE + 0x0300)
#define _CLK_PLL1_NCNT         0x0000
#define _CLK_PLL1_MCNT         0x0004
#define _CLK_PLL1_KCNT         0x0008
#define _CLK_PLL1_CTRL         0x000c
#define _CLK_PLL2_NCNT         0x0010
#define _CLK_PLL2_MCNT         0x0014
#define _CLK_PLL2_KCNT         0x0018
#define _CLK_PLL2_CTRL         0x001c
#define _CLK_DERIVE            0x0020
#define _CLK_STATUS            0x0024
#define _CLK_AHB1_COUNT        0x0028

#define _CLK_PLL1_CTRL_P    0x00000001
#define _CLK_PLL2_CTRL_P    0x00000001

#define _CLK_DERIVE_BP1     (1<<12)
#define _CLK_DERIVE_BP2     (1<<13)

#define _CLK_STATUS_L1       0x00000001
#define _CLK_STATUS_L2       0x00000002
#define _CLK_STATUS_C1       0x00000004
#define _CLK_STATUS_C2       0x00000008

// Settings from Altera example code. Note that this differs from the
// magic values described in the manual. I think the values are
// supposed to disable the PLLs, making the core run at 25MHz and
// peripherals at 12.5MHz
#define _CLK_PLL1_CTRL_INIT    0x00001064
#define _CLK_PLL2_CTRL_INIT    0x00001064
#define _CLK_DERIVE_INIT       0x00003010

//-----------------------------------------------------------------------------
// Expansion Bus Interface
#define INNOVATOR_EBI_CR       (INNOVATOR_BASE + 0x0380)

#define INNOVATOR_EBI_CR_EO    0x00000008

#define INNOVATOR_EBI_CR_INIT  (INNOVATOR_EBI_CR_EO)

//-----------------------------------------------------------------------------
// SDRAM controller
#define INNOVATOR_SDRAM_BASE   (INNOVATOR_BASE + 0x0400)
#define _SDRAM_TIMING1         0x0000
#define _SDRAM_TIMING2         0x0004
#define _SDRAM_CONFIG          0x0008
#define _SDRAM_REFRESH         0x000c
#define _SDRAM_ADDR            0x0010
#define _SDRAM_INIT            0x001c
#define _SDRAM_MODE0           0x0020
#define _SDRAM_MODE1           0x0024
#define INNOVATOR_SDRAM_WIDTH  (INNOVATOR_BASE + 0x007c)


#define _SDRAM_WIDTH_W        0x00000002
#define _SDRAM_WIDTH_LK       0x00000001

#if 0
// Max delays
#define _SDRAM_TIMING1_INIT    0x00009124
// CAS-2, 8 words burst, 3 clock refresh
#define _SDRAM_TIMING2_INIT    0x00000788
// SDR
#define _SDRAM_CONFIG_INIT     0x00000000
// Refresh period of 15us - at a clock of 75MHz that's 1125 cycles
#define _SDRAM_REFRESH_INIT    1125
// Rows (12) & columns (10)
#define _SDRAM_ADDR_INIT       0x0000ca80
// SDRAM mode (from Micron MT48LC16M8A2 manual)
//  M0-2: burst length     = 3 (8 words)
//  M3  : burst type       = 0 (sequential vs interleaved)
//  M4-6: CAS latency      = 2
//  M7-8: operating mode   = 0
//  M9  : write burst mode = 0 (writes are also in burst)
#define _SDRAM_MODE0_INIT      ((2<<4)|3)
// Unused (for DDR)
#define _SDRAM_MODE1_INIT      0x00000000
#endif

#define _SDRAM_INIT_EN      0x00008000
#define _SDRAM_INIT_PR      0x00004000
#define _SDRAM_INIT_LM      0x00002000
#define _SDRAM_INIT_LEM     0x00001000
#define _SDRAM_INIT_RF      0x00000800
#define _SDRAM_INIT_BS      0x00000400
#define _SDRAM_INIT_SR      0x00000200

//-----------------------------------------------------------------------------
// Watchdog controller
#define INNOVATOR_WDOG_CR               (INNOVATOR_BASE+0x0a00)
#define INNOVATOR_WDOG_COUNT            (INNOVATOR_BASE+0x0a04)
#define INNOVATOR_WDOG_RELOAD           (INNOVATOR_BASE+0x0a08)

//-----------------------------------------------------------------------------
// Interrupt controller
#define INNOVATOR_INT_MASK_SET          (INNOVATOR_BASE+0x0c00)
#define INNOVATOR_INT_MASK_CLEAR        (INNOVATOR_BASE+0x0c04)
#define INNOVATOR_INT_SOURCE_STATUS     (INNOVATOR_BASE+0x0c08)
#define INNOVATOR_INT_REQUEST_STATUS    (INNOVATOR_BASE+0x0c0c)
#define INNOVATOR_INT_ID                (INNOVATOR_BASE+0x0c10)
#define INNOVATOR_INT_PLD_PRIORITY      (INNOVATOR_BASE+0x0c14)
#define INNOVATOR_INT_INT_MODE          (INNOVATOR_BASE+0x0c18)
#define INNOVATOR_INT_PRIORITY_0        (INNOVATOR_BASE+0x0c80)
#define INNOVATOR_INT_PRIORITY_1        (INNOVATOR_BASE+0x0c84)
#define INNOVATOR_INT_PRIORITY_2        (INNOVATOR_BASE+0x0c88)
#define INNOVATOR_INT_PRIORITY_3        (INNOVATOR_BASE+0x0c8c)
#define INNOVATOR_INT_PRIORITY_4        (INNOVATOR_BASE+0x0c90)
#define INNOVATOR_INT_PRIORITY_5        (INNOVATOR_BASE+0x0c94)
#define INNOVATOR_INT_PRIORITY_6        (INNOVATOR_BASE+0x0c98)
#define INNOVATOR_INT_PRIORITY_7        (INNOVATOR_BASE+0x0c9c)
#define INNOVATOR_INT_PRIORITY_8        (INNOVATOR_BASE+0x0ca0)
#define INNOVATOR_INT_PRIORITY_9        (INNOVATOR_BASE+0x0ca4)
#define INNOVATOR_INT_PRIORITY_10       (INNOVATOR_BASE+0x0ca8)
#define INNOVATOR_INT_PRIORITY_11       (INNOVATOR_BASE+0x0cac)
#define INNOVATOR_INT_PRIORITY_12       (INNOVATOR_BASE+0x0cb0)
#define INNOVATOR_INT_PRIORITY_13       (INNOVATOR_BASE+0x0cb4)
#define INNOVATOR_INT_PRIORITY_14       (INNOVATOR_BASE+0x0cb8)
#define INNOVATOR_INT_PRIORITY_15       (INNOVATOR_BASE+0x0cbc)
#define INNOVATOR_INT_PRIORITY_16       (INNOVATOR_BASE+0x0cc0)

#define INNOVATOR_INT_SOURCE_P0         0x00000001
#define INNOVATOR_INT_SOURCE_P1         0x00000002
#define INNOVATOR_INT_SOURCE_P2         0x00000004
#define INNOVATOR_INT_SOURCE_P3         0x00000008
#define INNOVATOR_INT_SOURCE_P4         0x00000010
#define INNOVATOR_INT_SOURCE_P5         0x00000020
#define INNOVATOR_INT_SOURCE_IP         0x00000040
#define INNOVATOR_INT_SOURCE_UA         0x00000080
#define INNOVATOR_INT_SOURCE_T0         0x00000100
#define INNOVATOR_INT_SOURCE_T1         0x00000200
#define INNOVATOR_INT_SOURCE_PS         0x00000400
#define INNOVATOR_INT_SOURCE_EE         0x00000800
#define INNOVATOR_INT_SOURCE_PE         0x00001000
#define INNOVATOR_INT_SOURCE_AE         0x00002000
#define INNOVATOR_INT_SOURCE_CT         0x00004000
#define INNOVATOR_INT_SOURCE_CR         0x00008000
#define INNOVATOR_INT_SOURCE_FC         0x00010000

#define INNOVATOR_INT_PRIORITY_FIQ      0x00000040
#define INNOVATOR_INT_PRIORITY_LVL_mask 0x0000003f

//-----------------------------------------------------------------------------
// PLD
#define INNOVATOR_PLD_BASE     INNOVATOR_PLD_PHYS_BASE

#define INNOVATOR_PLD_LEDS     (INNOVATOR_PLD_BASE + 0x0100)


#endif
#endif // CYGONCE_INNOVATOR_H
//-----------------------------------------------------------------------------
// end of innovator.h
