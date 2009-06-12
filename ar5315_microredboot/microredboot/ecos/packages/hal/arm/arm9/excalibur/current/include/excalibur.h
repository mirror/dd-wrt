#ifndef CYGONCE_EXCALIBUR_H
#define CYGONCE_EXCALIBUR_H

//=============================================================================
//
//      excalibur.h
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
// Author(s):    jskov
// Contributors: jskov
// Date:         2001-08-06
// Purpose:      Altera/EXCALIBUR platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/excalibur.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal_arm_arm9_excalibur.h>

#define EXCALIBUR_BASE         EXCALIBUR_REGS_PHYS_BASE

//-----------------------------------------------------------------------------
// Boot control
// Note: this register is actually write-bit-to-clear-it
#define EXCALIBUR_BOOT_CR      (EXCALIBUR_BASE + 0x0000)

#define EXCALIBUR_BOOT_CR_BM   0x00000001
#define EXCALIBUR_BOOT_CR_HM   0x00000002
#define EXCALIBUR_BOOT_CR_RE   0x00000004

//-----------------------------------------------------------------------------
// DPSRAM config
#define EXCALIBUR_DPSRAM_BASE  (EXCALIBUR_BASE + 0x0030)
#define _DPSRAM0_SR            0x0000
#define _DPSRAM0_LCR           0x0004
#define _DPSRAM1_SR            0x0008
#define _DPSRAM1_LCR           0x000c

#define _DPSRAM0_LCR_INIT      0x00000000
#define _DPSRAM1_LCR_INIT      0x00000000

//-----------------------------------------------------------------------------
// IO controller
#define EXCALIBUR_IOCR_BASE    (EXCALIBUR_BASE + 0x0040)
#define _IOCR_SDRAM            0x0000
#define _IOCR_EBI              0x0004
#define _IOCR_UART             0x0008
#define _IOCR_TRACE            0x000c

#define _IOCR_OC_PCI           0x00000008
#define _IOCR_OC_FAST          0x00000004
#define _IOCR_OC_SLOW          0x00000000
#define _IOCR_IO_STRIPE        0x00000002
#define _IOCR_LOCK             0x00000001

#define EXCALIBUR_IOCR_SDRAM_INIT (_IOCR_OC_FAST | _IOCR_IO_STRIPE | _IOCR_LOCK)
#define EXCALIBUR_IOCR_EBI_INIT   (_IOCR_OC_SLOW | _IOCR_IO_STRIPE | _IOCR_LOCK)
#define EXCALIBUR_IOCR_UART_INIT  (_IOCR_OC_SLOW | _IOCR_IO_STRIPE | _IOCR_LOCK)


//-----------------------------------------------------------------------------
// Memory mapping
#define EXCALIBUR_MMAP_BASE    (EXCALIBUR_BASE + 0x0080)
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

#define _MMAP_REGISTERS_INIT   (EXCALIBUR_REGS_PHYS_BASE  + 0x00000000 | _MMAP_SIZE_16K  | _MMAP_NOPREFETCH | _MMAP_ENABLE)
#define _MMAP_SRAM0_INIT       (EXCALIBUR_SRAM_PHYS_BASE  + 0x00000000 | _MMAP_SIZE_128K | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_SRAM1_INIT       (EXCALIBUR_SRAM_PHYS_BASE  + 0x00020000 | _MMAP_SIZE_128K | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_DPSRAM0_INIT     (EXCALIBUR_SRAM_PHYS_BASE  + 0x00040000 | _MMAP_SIZE_64K  | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_DPSRAM1_INIT     (EXCALIBUR_SRAM_PHYS_BASE  + 0x00050000 | _MMAP_SIZE_64K  | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_SDRAM0_INIT      (EXCALIBUR_SDRAM_PHYS_BASE + 0x00000000 | _MMAP_SIZE_64M  | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_SDRAM1_INIT      (EXCALIBUR_SDRAM_PHYS_BASE + 0x04000000 | _MMAP_SIZE_64M  | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_EBI0_INIT        (EXCALIBUR_FLASH_PHYS_BASE + 0x00000000 | _MMAP_SIZE_4M   | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_EBI1_INIT        (EXCALIBUR_FLASH_PHYS_BASE + 0x00400000 | _MMAP_SIZE_4M   | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_EBI2_INIT        (EXCALIBUR_FLASH_PHYS_BASE + 0x00800000 | _MMAP_SIZE_4M   | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_EBI3_INIT        (EXCALIBUR_FLASH_PHYS_BASE + 0x00c00000 | _MMAP_SIZE_4M   | _MMAP_PREFETCH   | _MMAP_ENABLE)
#define _MMAP_PLD0_INIT        (EXCALIBUR_PLD_PHYS_BASE   + 0x00000000 | _MMAP_SIZE_16K | _MMAP_NOPREFETCH | _MMAP_ENABLE)
//#define _MMAP_PLD1_INIT        (EXCALIBUR_PLD_PHYS_BASE   + 0x00004000 | _MMAP_SIZE_16K | _MMAP_NOPREFETCH | _MMAP_ENABLE)
#define _MMAP_PLD1_INIT        (0x0f000000 | _MMAP_SIZE_16K | _MMAP_NOPREFETCH | _MMAP_ENABLE)
#define _MMAP_PLD2_INIT        (EXCALIBUR_PLD_PHYS_BASE   + 0x00008000 | _MMAP_SIZE_16K | _MMAP_NOPREFETCH | _MMAP_ENABLE)
#define _MMAP_PLD3_INIT        (EXCALIBUR_PLD_PHYS_BASE   + 0x0000c000 | _MMAP_SIZE_16K | _MMAP_NOPREFETCH | _MMAP_ENABLE)

#define EXCALIBUR_SDRAM_PHYS_BASE 0x00000000
#define EXCALIBUR_FLASH_PHYS_BASE 0x40000000
#define EXCALIBUR_SRAM_PHYS_BASE  0x08000000
#define EXCALIBUR_PLD_PHYS_BASE   0x80000000
#define EXCALIBUR_REGS_PHYS_BASE  0x7fffc000




//-----------------------------------------------------------------------------
// Timers
#define EXCALIBUR_TIMER0_CR              (EXCALIBUR_BASE+0x0200)
#define EXCALIBUR_TIMER0_PRE             (EXCALIBUR_BASE+0x0210)
#define EXCALIBUR_TIMER0_LIMIT           (EXCALIBUR_BASE+0x0220)
#define EXCALIBUR_TIMER0_READ            (EXCALIBUR_BASE+0x0230)

#define EXCALIBUR_TIMER1_CR              (EXCALIBUR_BASE+0x0240)
#define EXCALIBUR_TIMER1_PRE             (EXCALIBUR_BASE+0x0250)
#define EXCALIBUR_TIMER1_LIMIT           (EXCALIBUR_BASE+0x0260)
#define EXCALIBUR_TIMER1_READ            (EXCALIBUR_BASE+0x0270)

#define EXCALIBUR_TIMER_CR_MODE_HEARBEAT 0x00000000
#define EXCALIBUR_TIMER_CR_MODE_ONE_SHOT 0x00000001
#define EXCALIBUR_TIMER_CR_IE            0x00000004
#define EXCALIBUR_TIMER_CR_CI            0x00000008
#define EXCALIBUR_TIMER_CR_S             0x00000010

//-----------------------------------------------------------------------------
// Serial
#define EXCALIBUR_UART0_BASE            (EXCALIBUR_BASE+0x0280)
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
#define EXCALIBUR_CLK_BASE     (EXCALIBUR_BASE + 0x0300)
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
#define EXCALIBUR_EBI_CR       (EXCALIBUR_BASE + 0x0380)

#define EXCALIBUR_EBI_CR_EO    0x00000008

#define EXCALIBUR_EBI_CR_INIT  (EXCALIBUR_EBI_CR_EO)

//-----------------------------------------------------------------------------
// SDRAM controller
#define EXCALIBUR_SDRAM_BASE   (EXCALIBUR_BASE + 0x0400)
#define _SDRAM_TIMING1         0x0000
#define _SDRAM_TIMING2         0x0004
#define _SDRAM_CONFIG          0x0008
#define _SDRAM_REFRESH         0x000c
#define _SDRAM_ADDR            0x0010
#define _SDRAM_INIT            0x001c
#define _SDRAM_MODE0           0x0020
#define _SDRAM_MODE1           0x0024
#define EXCALIBUR_SDRAM_WIDTH  (EXCALIBUR_BASE + 0x007c)


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

#if (CYGNUM_HAL_ARM_EXCALIBUR_SDRAM_CLOCK != 75000000)
# error "Hardwired for a 75MHz SDRAM clock"
#endif

//-----------------------------------------------------------------------------
// Watchdog controller
#define EXCALIBUR_WDOG_CR               (EXCALIBUR_BASE+0x0a00)
#define EXCALIBUR_WDOG_COUNT            (EXCALIBUR_BASE+0x0a04)
#define EXCALIBUR_WDOG_RELOAD           (EXCALIBUR_BASE+0x0a08)

//-----------------------------------------------------------------------------
// Interrupt controller
#define EXCALIBUR_INT_MASK_SET          (EXCALIBUR_BASE+0x0c00)
#define EXCALIBUR_INT_MASK_CLEAR        (EXCALIBUR_BASE+0x0c04)
#define EXCALIBUR_INT_SOURCE_STATUS     (EXCALIBUR_BASE+0x0c08)
#define EXCALIBUR_INT_REQUEST_STATUS    (EXCALIBUR_BASE+0x0c0c)
#define EXCALIBUR_INT_ID                (EXCALIBUR_BASE+0x0c10)
#define EXCALIBUR_INT_PLD_PRIORITY      (EXCALIBUR_BASE+0x0c14)
#define EXCALIBUR_INT_INT_MODE          (EXCALIBUR_BASE+0x0c18)
#define EXCALIBUR_INT_PRIORITY_0        (EXCALIBUR_BASE+0x0c80)
#define EXCALIBUR_INT_PRIORITY_1        (EXCALIBUR_BASE+0x0c84)
#define EXCALIBUR_INT_PRIORITY_2        (EXCALIBUR_BASE+0x0c88)
#define EXCALIBUR_INT_PRIORITY_3        (EXCALIBUR_BASE+0x0c8c)
#define EXCALIBUR_INT_PRIORITY_4        (EXCALIBUR_BASE+0x0c90)
#define EXCALIBUR_INT_PRIORITY_5        (EXCALIBUR_BASE+0x0c94)
#define EXCALIBUR_INT_PRIORITY_6        (EXCALIBUR_BASE+0x0c98)
#define EXCALIBUR_INT_PRIORITY_7        (EXCALIBUR_BASE+0x0c9c)
#define EXCALIBUR_INT_PRIORITY_8        (EXCALIBUR_BASE+0x0ca0)
#define EXCALIBUR_INT_PRIORITY_9        (EXCALIBUR_BASE+0x0ca4)
#define EXCALIBUR_INT_PRIORITY_10       (EXCALIBUR_BASE+0x0ca8)
#define EXCALIBUR_INT_PRIORITY_11       (EXCALIBUR_BASE+0x0cac)
#define EXCALIBUR_INT_PRIORITY_12       (EXCALIBUR_BASE+0x0cb0)
#define EXCALIBUR_INT_PRIORITY_13       (EXCALIBUR_BASE+0x0cb4)
#define EXCALIBUR_INT_PRIORITY_14       (EXCALIBUR_BASE+0x0cb8)
#define EXCALIBUR_INT_PRIORITY_15       (EXCALIBUR_BASE+0x0cbc)
#define EXCALIBUR_INT_PRIORITY_16       (EXCALIBUR_BASE+0x0cc0)

#define EXCALIBUR_INT_SOURCE_P0         0x00000001
#define EXCALIBUR_INT_SOURCE_P1         0x00000002
#define EXCALIBUR_INT_SOURCE_P2         0x00000004
#define EXCALIBUR_INT_SOURCE_P3         0x00000008
#define EXCALIBUR_INT_SOURCE_P4         0x00000010
#define EXCALIBUR_INT_SOURCE_P5         0x00000020
#define EXCALIBUR_INT_SOURCE_IP         0x00000040
#define EXCALIBUR_INT_SOURCE_UA         0x00000080
#define EXCALIBUR_INT_SOURCE_T0         0x00000100
#define EXCALIBUR_INT_SOURCE_T1         0x00000200
#define EXCALIBUR_INT_SOURCE_PS         0x00000400
#define EXCALIBUR_INT_SOURCE_EE         0x00000800
#define EXCALIBUR_INT_SOURCE_PE         0x00001000
#define EXCALIBUR_INT_SOURCE_AE         0x00002000
#define EXCALIBUR_INT_SOURCE_CT         0x00004000
#define EXCALIBUR_INT_SOURCE_CR         0x00008000
#define EXCALIBUR_INT_SOURCE_FC         0x00010000

#define EXCALIBUR_INT_PRIORITY_FIQ      0x00000040
#define EXCALIBUR_INT_PRIORITY_LVL_mask 0x0000003f

//-----------------------------------------------------------------------------
// PLD
#define EXCALIBUR_PLD_BASE     EXCALIBUR_PLD_PHYS_BASE

#define EXCALIBUR_PLD_LEDS     (EXCALIBUR_PLD_BASE + 0x0100)



#endif // CYGONCE_EXCALIBUR_H
//-----------------------------------------------------------------------------
// end of excalibur.h
