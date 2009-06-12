//==========================================================================
//
//      frv400.h
//
//      HAL misc board support definitions for Fujitsu MB93091 (FR-V 400)
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2001-09-07
// Purpose:      Platform register definitions
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#ifndef __HAL_FRV400_H__
#define __HAL_FRV400_H__ 1

// Common

// Processor status register
#define _PSR_PIVL_SHIFT 3
#define _PSR_PIVL_MASK  (0xF<<(_PSR_PIVL_SHIFT))  // Interrupt mask level
#define _PSR_S          (1<<2)                    // Supervisor state
#define _PSR_PS         (1<<1)                    // Previous supervisor state
#define _PSR_ET         (1<<0)                    // Enable interrupts

// Hardware status register
#define _HSR0_ICE       (1<<31)                   // Instruction cache enable
#define _HSR0_DCE       (1<<30)                   // Data cache enable
#define _HSR0_IMMU      (1<<26)                   // Instruction MMU enable
#define _HSR0_DMMU      (1<<25)                   // Data MMU enable

// Debug Control Register
#define _DCR_EBE        (1 << 30) // Exception break enable bit 
#define _DCR_SE         (1 << 29) // Single-step break enable bit 
#define _DCR_IBM        (1 << 28) // Instruction Break Mask (disable bit)
#define _DCR_DRBE0      (1 << 19) // READ dbar0
#define _DCR_DWBE0      (1 << 18) // WRITE dbar0
#define _DCR_DDBE0      (1 << 17) // Data-match for access to dbar0
#define _DCR_DBASE0     (1 << 17) // offset
#define _DCR_DRBE1      (1 << 16)
#define _DCR_DWBE1      (1 << 15)
#define _DCR_DDBE1      (1 << 14)
#define _DCR_DBASE1     (1 << 14)
//#define _DCR_DRBE2      (1 << 13) // 2 and 3 not supported in real hardware
//#define _DCR_DWBE2      (1 << 12)
//#define _DCR_DDBE2      (1 << 11)
//#define _DCR_DRBE3      (1 << 10)
//#define _DCR_DWBE3      (1 << 9)
//#define _DCR_DDBE3      (1 << 8)
#define _DCR_IBE0       (1 << 7)
#define _DCR_IBCE0      (1 << 6)
#define _DCR_IBE1       (1 << 5)
#define _DCR_IBCE1      (1 << 4)
#define _DCR_IBE2       (1 << 3)
#define _DCR_IBCE2      (1 << 2)
#define _DCR_IBE3       (1 << 1)
#define _DCR_IBCE3      (1 << 0)

// Break PSR Save Register
#define _BPSR_BS	(1 << 12)
#define _BPSR_BET	(1 << 0)

// Break Request Register
#define _BRR_EB         (1 << 30)
#define _BRR_CB         (1 << 29)
#define _BRR_TB         (1 << 28)
#define _BRR_DB0        (1 << 11)
#define _BRR_DB1        (1 << 10)
#define _BRR_IB0        (1 << 7)
#define _BRR_IB1        (1 << 6)
#define _BRR_IB2        (1 << 5)
#define _BRR_IB3        (1 << 4)
#define _BRR_CBB        (1 << 3)
#define _BRR_BB         (1 << 2)
#define _BRR_SB         (1 << 1)
#define _BRR_ST         (1 << 0)


// Platform specifics

// SDRAM Controller
#define _FRV400_SDRAM_CP  0xFE000400              // Controller protect
#define _FRV400_SDRAM_CFG 0xFE000410              // Configuration
#define _FRV400_SDRAM_CTL 0xFE000418              // Control
#define _FRV400_SDRAM_MS  0xFE000420              // Mode select
#define _FRV400_SDRAM_STS 0xFE000428              // Status
#define _FRV400_SDRAM_RCN 0xFE000430              // Refresh control
#define _FRV400_SDRAM_ART 0xFE000438              // Auto-refresh timer
#define _FRV400_SDRAM_AN0 0xFE000500              // Address #0
#define _FRV400_SDRAM_AN1 0xFE000508              // Address #1
#define _FRV400_SDRAM_BR0 0xFE000E00              // Base register #0
#define _FRV400_SDRAM_BR1 0xFE000E08              // Base register #1
#define _FRV400_SDRAM_AM0 0xFE000F00              // Address mask #0
#define _FRV400_SDRAM_AM1 0xFE000F08              // Address mask #1

// Local bus control
#define _FRV400_LBUS_CP   0xFE000000              // Controller protect
#define _FRV400_LBUS_EST  0xFE000020              // Error status
#define _FRV400_LBUS_EAD  0xFE000028              // Error address
#define _FRV400_LBUS_CR0  0xFE000100              // Configuration - space #0
#define _FRV400_LBUS_CR1  0xFE000108              // Configuration - space #1
#define _FRV400_LBUS_CR2  0xFE000110              // Configuration - space #2
#define _FRV400_LBUS_CR3  0xFE000118              // Configuration - space #3
#define _FRV400_LBUS_BR0  0xFE000C00              // Slave - base address #0
#define _FRV400_LBUS_BR1  0xFE000C08              // Slave - base address #1
#define _FRV400_LBUS_BR2  0xFE000C10              // Slave - base address #2
#define _FRV400_LBUS_BR3  0xFE000C18              // Slave - base address #3
#define _FRV400_LBUS_AM0  0xFE000D00              // Slave - address mask #0
#define _FRV400_LBUS_AM1  0xFE000D08              // Slave - address mask #1
#define _FRV400_LBUS_AM2  0xFE000D10              // Slave - address mask #2
#define _FRV400_LBUS_AM3  0xFE000D18              // Slave - address mask #3

// PCI Bridge (on motherboard)
#define _FRV400_PCI_SLBUS_CONFIG       0x10000000
#define _FRV400_PCI_ECS0_CONFIG        0x10000008
#define _FRV400_PCI_ECS1_CONFIG        0x10000010
#define _FRV400_PCI_ECS2_CONFIG        0x10000018
#define _FRV400_PCI_ECS0_RANGE         0x10000020
#define _FRV400_PCI_ECS0_ADDR          0x10000028
#define _FRV400_PCI_ECS1_RANGE         0x10000030
#define _FRV400_PCI_ECS1_ADDR          0x10000038
#define _FRV400_PCI_ECS2_RANGE         0x10000040
#define _FRV400_PCI_ECS2_ADDR          0x10000048
#define _FRV400_PCI_PCIIO_RANGE        0x10000050
#define _FRV400_PCI_PCIIO_ADDR         0x10000058
#define _FRV400_PCI_PCIMEM_RANGE       0x10000060
#define _FRV400_PCI_PCIMEM_ADDR        0x10000068
#define _FRV400_PCI_PCIIO_PCI_ADDR     0x10000070
#define _FRV400_PCI_PCIMEM_PCI_ADDR    0x10000078
#define _FRV400_PCI_CONFIG_ADDR        0x10000080
#define _FRV400_PCI_CONFIG_DATA        0x10000088

#define _FRV400_PCI_SL_TO_PCI_MBX0     0x10000500
#define _FRV400_PCI_SL_TO_PCI_MBX1     0x10000508
#define _FRV400_PCI_SL_TO_PCI_MBX2     0x10000510
#define _FRV400_PCI_SL_TO_PCI_MBX3     0x10000518
#define _FRV400_PCI_SL_TO_PCI_MBX4     0x10000520
#define _FRV400_PCI_SL_TO_PCI_MBX5     0x10000528
#define _FRV400_PCI_SL_TO_PCI_MBX6     0x10000530
#define _FRV400_PCI_SL_TO_PCI_MBX7     0x10000538
#define _FRV400_PCI_PCI_TO_SL_MBX0     0x10000540
#define _FRV400_PCI_PCI_TO_SL_MBX1     0x10000548
#define _FRV400_PCI_PCI_TO_SL_MBX2     0x10000550
#define _FRV400_PCI_PCI_TO_SL_MBX3     0x10000558
#define _FRV400_PCI_PCI_TO_SL_MBX4     0x10000560
#define _FRV400_PCI_PCI_TO_SL_MBX5     0x10000568
#define _FRV400_PCI_PCI_TO_SL_MBX6     0x10000570
#define _FRV400_PCI_PCI_TO_SL_MBX7     0x10000578
#define _FRV400_PCI_MBX_STATUS         0x10000580
#define _FRV400_PCI_MBX_CONTROL        0x10000588
#define _FRV400_PCI_SL_TO_PCI_DOORBELL 0x10000590
#define _FRV400_PCI_PCI_TO_SL_DOORBELL 0x10000598
#define _FRV400_PCI_SL_INT_STATUS      0x100005A0
#define _FRV400_PCI_SL_INT_STATUS_MASTER_ABORT (1<<26)
#define _FRV400_PCI_PCI_INT_STATUS     0x100005A8
#define _FRV400_PCI_SL_INT_ENABLE      0x100005B0
#define _FRV400_PCI_PCI_INT_ENABLE     0x100005B8

#define _FRV400_PCI_CONFIG             0x10000800
#define _FRV400_PCI_DEVICE_VENDOR      0x10000800
#define _FRV400_PCI_STAT_CMD           0x10000808
#define _FRV400_PCI_STAT_ERROR_MASK      0xF000
#define _FRV400_PCI_CLASS_REV          0x10000810
#define _FRV400_PCI_BIST               0x10000818
#define _FRV400_PCI_PCI_IO_MAPPED      0x10000820
#define _FRV400_PCI_PCI_MEM_MAP_LO     0x10000828
#define _FRV400_PCI_PCI_ECS0_LO        0x10000838
#define _FRV400_PCI_PCI_ECS1_LO        0x10000840
#define _FRV400_PCI_PCI_ECS2_LO        0x10000848
#define _FRV400_PCI_MAX_LAT            0x10000878
#define _FRV400_PCI_TMO_RETRY          0x10000880
#define _FRV400_PCI_SERR_ENABLE        0x10000888
#define _FRV400_PCI_RESET              0x10000890
#define _FRV400_PCI_RESET_SRST           0x00000001  // Assert soft reset
#define _FRV400_PCI_PCI_MEM_MAP_HI     0x10000898
#define _FRV400_PCI_PCI_ECS0_HI        0x100008A8
#define _FRV400_PCI_PCI_ECS1_HI        0x100008B0
#define _FRV400_PCI_PCI_ECS2_HI        0x100008B8

// Motherboard resources
#define _FRV400_MB_SWGP                0x21200000   // General purpose switches
#define _FRV400_MB_LEDS                0x21200004   // LED array - 16 bits 0->on
#define _FRV400_MB_LCD                 0x21200008   // LCD panel
#define _FRV400_MB_BOOT_MODE           0x21300004   // Boot mode register
#define _FRV400_MB_H_RESET             0x21300008   // Hardware reset
#define _FRV400_MB_CLKSW               0x2130000C   // Clock settings
#define _FRV400_MB_PCI_ARBITER         0x21300014   // Enable PCI arbiter mode

// Serial ports - 16550 compatible
#define _FRV400_UART0   0xFEFF9C00
#define _FRV400_UART1   0xFEFF9C40

// Programmable timers
#define _FRV400_TCSR0   0xFEFF9400                // Timer 0 control/status
#define _FRV400_TCSR1   0xFEFF9408                // Timer 1 control/status
#define _FRV400_TCSR2   0xFEFF9410                // Timer 2 control/status
#define _FRV400_TCxSR_TOUT 0x80                     // Status - TOUT signal 
#define _FRV400_TCTR    0xFEFF9418                // Timer control
#define _FRV400_TCTR_SEL0 (0<<6)                    // Select timer 0
#define _FRV400_TCTR_SEL1 (1<<6)                    // Select timer 1
#define _FRV400_TCTR_SEL2 (2<<6)                    // Select timer 2
#define _FRV400_TCTR_RB   (3<<6)                    // Timer read back
#define _FRV400_TCTR_RB_NCOUNT   (1<<5)                    // Count data suppress
#define _FRV400_TCTR_RB_NSTATUS  (1<<4)                    // Status data suppress
#define _FRV400_TCTR_RB_CTR2     (1<<3)                    // Read data for counter #2
#define _FRV400_TCTR_RB_CTR1     (1<<2)                    // Read data for counter #1
#define _FRV400_TCTR_RB_CTR0     (1<<1)                    // Read data for counter #0
#define _FRV400_TCTR_LATCH (0<<4)                   // Counter latch command
#define _FRV400_TCTR_R8LO  (1<<4)                   // Read low 8 bits
#define _FRV400_TCTR_R8HI  (2<<4)                   // Read high 8 bits
#define _FRV400_TCTR_RLOHI (3<<4)                   // Read/write 8 lo then 8 hi
#define _FRV400_TCTR_MODE0 (0<<1)                   // Mode 0 - terminal interrupt count
#define _FRV400_TCTR_MODE2 (2<<1)                   // Mode 2 - rate generator
#define _FRV400_TCTR_MODE4 (4<<1)                   // Mode 4 - software trigger strobe
#define _FRV400_TCTR_MODE5 (5<<1)                   // Mode 5 - hardware trigger strobe
#define _FRV400_TPRV    0xFEFF9420                // Timer prescale
#define _FRV400_TPRCKSL 0xFEFF9428                // Prescale clock
#define _FRV400_TCKSL0  0xFEFF9430                // Timer 0 clock select
#define _FRV400_TCKSL1  0xFEFF9438                // Timer 1 clock select
#define _FRV400_TCKSL2  0xFEFF9440                // Timer 2 clock select

// Interrupt & clock control
#define _FRV400_CLK_CTRL 0xFEFF9A00               // Clock control
#define _FRV400_IRC_TM0  0xFEFF9800               // Trigger mode 0 register (unused)
#define _FRV400_IRC_TM1  0xFEFF9808               // Trigger mode 1 register
#define _FRV400_IRC_RS   0xFEFF9810               // Request sense
#define _FRV400_IRC_RC   0xFEFF9818               // Request clear
#define _FRV400_IRC_MASK 0xFEFF9820               // Mask
#define _FRV400_IRC_IRL  0xFEFF9828               // Interrupt level read (encoded)
#define _FRV400_IRC_IRR0 0xFEFF9840               // Interrupt routing #0 (unused)
#define _FRV400_IRC_IRR1 0xFEFF9848               // Interrupt routing #1 (unused)
#define _FRV400_IRC_IRR2 0xFEFF9850               // Interrupt routing #2 (unused)
#define _FRV400_IRC_IRR3 0xFEFF9858               // Interrupt routing #3
#define _FRV400_IRC_IRR4 0xFEFF9860               // Interrupt routing #4
#define _FRV400_IRC_IRR5 0xFEFF9868               // Interrupt routing #5
#define _FRV400_IRC_IRR6 0xFEFF9870               // Interrupt routing #6
#define _FRV400_IRC_IRR7 0xFEFF9878               // Interrupt routing #7
#define _FRV400_IRC_ITM0 0xFEFF9880               // Internal trigger mode #0
#define _FRV400_IRC_ITM1 0xFEFF9888               // Internal trigger mode #1

// Some GPIO magic
#define	_FRV400_GPIO_SIR 0xFEFF0410               // Special input signals
#define	_FRV400_GPIO_SOR 0xFEFF0418               // Special output signals

// Reset register
#define _FRV400_HW_RESET 0xFEFF0500               // Hardware reset
#define _FRV400_HW_RESET_STAT_P (1<<10)              // Last reset was power-on
#define _FRV400_HW_RESET_STAT_H (1<<9)               // Last reset was hard reset
#define _FRV400_HW_RESET_STAT_S (1<<8)               // Last reset was soft reset
#define _FRV400_HW_RESET_HR     (1<<1)               // Force hard reset
#define _FRV400_HW_RESET_SR     (1<<0)               // Force soft reset

// On-board FPGA
#define _FRV400_FPGA_CONTROL      0xFFC00000      // Access control for FPGA resources
#define _FRV400_FPGA_CONTROL_IRQ      (1<<2)        // Set to enable IRQ registers
#define _FRV400_FPGA_CONTROL_CS4      (1<<1)        // Set to enable CS4 control regs
#define _FRV400_FPGA_CONTROL_CS5      (1<<0)        // Set to enable CS5 control regs
#define _FRV400_FPGA_IRQ_MASK     0xFFC00004      // Set bits to 0 to allow interrupt
#define _FRV400_FPGA_IRQ_LEVELS   0xFFC00008      // 0=>active low, 1=>active high
#define _FRV400_FPGA_IRQ_REQUEST  0xFFC0000C      // read: 1=>asserted, write: 0=>clears
#define _FRV400_FPGA_IRQ_LAN         (1<<12)        // Onboard LAN controller
#define _FRV400_FPGA_IRQ_INTA         (1<<6)        // PCI bus INTA
#define _FRV400_FPGA_IRQ_INTB         (1<<5)        // PCI bus INTA
#define _FRV400_FPGA_IRQ_INTC         (1<<4)        // PCI bus INTA
#define _FRV400_FPGA_IRQ_INTD         (1<<3)        // PCI bus INTA

#endif /* __HAL_FRV400_H__ */
