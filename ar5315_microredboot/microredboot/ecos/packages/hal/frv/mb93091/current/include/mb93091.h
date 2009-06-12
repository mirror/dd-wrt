//==========================================================================
//
//      mb93091.h
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

#ifndef __HAL_MB93091_H__
#define __HAL_MB93091_H__ 1

#include <cyg/hal/fr-v.h>
#include <cyg/hal/fr400.h>
#include <cyg/hal/fr500.h>

// PCI Bridge (on motherboard)
#define _MB93091_PCI_SLBUS_CONFIG       0x10000000
#define _MB93091_PCI_ECS0_CONFIG        0x10000008
#define _MB93091_PCI_ECS1_CONFIG        0x10000010
#define _MB93091_PCI_ECS2_CONFIG        0x10000018
#define _MB93091_PCI_ECS0_RANGE         0x10000020
#define _MB93091_PCI_ECS0_ADDR          0x10000028
#define _MB93091_PCI_ECS1_RANGE         0x10000030
#define _MB93091_PCI_ECS1_ADDR          0x10000038
#define _MB93091_PCI_ECS2_RANGE         0x10000040
#define _MB93091_PCI_ECS2_ADDR          0x10000048
#define _MB93091_PCI_PCIIO_RANGE        0x10000050
#define _MB93091_PCI_PCIIO_ADDR         0x10000058
#define _MB93091_PCI_PCIMEM_RANGE       0x10000060
#define _MB93091_PCI_PCIMEM_ADDR        0x10000068
#define _MB93091_PCI_PCIIO_PCI_ADDR     0x10000070
#define _MB93091_PCI_PCIMEM_PCI_ADDR    0x10000078
#define _MB93091_PCI_CONFIG_ADDR        0x10000080
#define _MB93091_PCI_CONFIG_DATA        0x10000088

#define _MB93091_PCI_SL_TO_PCI_MBX0     0x10000500
#define _MB93091_PCI_SL_TO_PCI_MBX1     0x10000508
#define _MB93091_PCI_SL_TO_PCI_MBX2     0x10000510
#define _MB93091_PCI_SL_TO_PCI_MBX3     0x10000518
#define _MB93091_PCI_SL_TO_PCI_MBX4     0x10000520
#define _MB93091_PCI_SL_TO_PCI_MBX5     0x10000528
#define _MB93091_PCI_SL_TO_PCI_MBX6     0x10000530
#define _MB93091_PCI_SL_TO_PCI_MBX7     0x10000538
#define _MB93091_PCI_PCI_TO_SL_MBX0     0x10000540
#define _MB93091_PCI_PCI_TO_SL_MBX1     0x10000548
#define _MB93091_PCI_PCI_TO_SL_MBX2     0x10000550
#define _MB93091_PCI_PCI_TO_SL_MBX3     0x10000558
#define _MB93091_PCI_PCI_TO_SL_MBX4     0x10000560
#define _MB93091_PCI_PCI_TO_SL_MBX5     0x10000568
#define _MB93091_PCI_PCI_TO_SL_MBX6     0x10000570
#define _MB93091_PCI_PCI_TO_SL_MBX7     0x10000578
#define _MB93091_PCI_MBX_STATUS         0x10000580
#define _MB93091_PCI_MBX_CONTROL        0x10000588
#define _MB93091_PCI_SL_TO_PCI_DOORBELL 0x10000590
#define _MB93091_PCI_PCI_TO_SL_DOORBELL 0x10000598
#define _MB93091_PCI_SL_INT_STATUS      0x100005A0
#define _MB93091_PCI_SL_INT_STATUS_MASTER_ABORT (1<<26)
#define _MB93091_PCI_PCI_INT_STATUS     0x100005A8
#define _MB93091_PCI_SL_INT_ENABLE      0x100005B0
#define _MB93091_PCI_PCI_INT_ENABLE     0x100005B8

#define _MB93091_PCI_CONFIG             0x10000800
#define _MB93091_PCI_DEVICE_VENDOR      0x10000800
#define _MB93091_PCI_STAT_CMD           0x10000808
#define _MB93091_PCI_STAT_ERROR_MASK      0xF000
#define _MB93091_PCI_CLASS_REV          0x10000810
#define _MB93091_PCI_BIST               0x10000818
#define _MB93091_PCI_PCI_IO_MAPPED      0x10000820
#define _MB93091_PCI_PCI_MEM_MAP_LO     0x10000828
#define _MB93091_PCI_PCI_ECS0_LO        0x10000838
#define _MB93091_PCI_PCI_ECS1_LO        0x10000840
#define _MB93091_PCI_PCI_ECS2_LO        0x10000848
#define _MB93091_PCI_MAX_LAT            0x10000878
#define _MB93091_PCI_TMO_RETRY          0x10000880
#define _MB93091_PCI_SERR_ENABLE        0x10000888
#define _MB93091_PCI_RESET              0x10000890
#define _MB93091_PCI_RESET_SRST           0x00000001  // Assert soft reset
#define _MB93091_PCI_PCI_MEM_MAP_HI     0x10000898
#define _MB93091_PCI_PCI_ECS0_HI        0x100008A8
#define _MB93091_PCI_PCI_ECS1_HI        0x100008B0
#define _MB93091_PCI_PCI_ECS2_HI        0x100008B8

// Motherboard resources
#define _MB93091_MB_SWGP                0x21200000   // General purpose switches
#define _MB93091_MB_LEDS                0x21200004   // LED array - 16 bits 0->on
#define _MB93091_MB_LCD                 0x21200008   // LCD panel
#define _MB93091_MB_BOOT_MODE           0x21300004   // Boot mode register
#define _MB93091_MB_H_RESET             0x21300008   // Hardware reset
#define _MB93091_MB_CLKSW               0x2130000C   // Clock settings
#define _MB93091_MB_PCI_ARBITER         0x21300014   // Enable PCI arbiter mode

#define LCD_D           0x000000ff	/* LCD data bus */
#define LCD_RW          0x00000100	/* LCD R/W signal */
#define LCD_RS          0x00000200	/* LCD Register Select */
#define LCD_E           0x00000400	/* LCD Start Enable Signal */
                                                                                
#define LCD_CMD_CLEAR           (LCD_E|0x001)
#define LCD_CMD_HOME            (LCD_E|0x002)
#define LCD_CMD_CURSOR_INC      (LCD_E|0x004)
#define LCD_CMD_SCROLL_INC      (LCD_E|0x005)
#define LCD_CMD_CURSOR_DEC      (LCD_E|0x006)
#define LCD_CMD_SCROLL_DEC      (LCD_E|0x007)
#define LCD_CMD_OFF             (LCD_E|0x008)
#define LCD_CMD_ON(CRSR,BLINK)  (LCD_E|0x00c|(CRSR<<1)|BLINK)
#define LCD_CMD_CURSOR_MOVE_L   (LCD_E|0x010)
#define LCD_CMD_CURSOR_MOVE_R   (LCD_E|0x014)
#define LCD_CMD_DISPLAY_SHIFT_L (LCD_E|0x018)
#define LCD_CMD_DISPLAY_SHIFT_R (LCD_E|0x01c)
#define LCD_CMD_FUNCSET(DL,N,F) (LCD_E|0x020|(DL<<4)|(N<<3)|(F<<2))
#define LCD_CMD_SET_CG_ADDR(X)  (LCD_E|0x040|X)
#define LCD_CMD_SET_DD_ADDR(X)  (LCD_E|0x080|X)
#define LCD_CMD_READ_BUSY       (LCD_E|LCD_RW)
#define LCD_DATA_WRITE(X)       (LCD_E|LCD_RS|(X))
#define LCD_DATA_READ           (LCD_E|LCD_RS|LCD_RW)

// On-board FPGA
#define _MB93091_FPGA_CONTROL      0xFFC00000      // Access control for FPGA resources
#define _MB93091_FPGA_CONTROL_IRQ      (1<<2)        // Set to enable IRQ registers
#define _MB93091_FPGA_CONTROL_CS4      (1<<1)        // Set to enable CS4 control regs
#define _MB93091_FPGA_CONTROL_CS5      (1<<0)        // Set to enable CS5 control regs
#define _MB93091_FPGA_IRQ_MASK     0xFFC00004      // Set bits to 0 to allow interrupt
#define _MB93091_FPGA_IRQ_LEVELS   0xFFC00008      // 0=>active low, 1=>active high
#define _MB93091_FPGA_IRQ_REQUEST  0xFFC0000C      // read: 1=>asserted, write: 0=>clears
#define _MB93091_FPGA_IRQ_LAN         (1<<12)        // Onboard LAN controller
#define _MB93091_FPGA_IRQ_INTA         (1<<6)        // PCI bus INTA
#define _MB93091_FPGA_IRQ_INTB         (1<<5)        // PCI bus INTA
#define _MB93091_FPGA_IRQ_INTC         (1<<4)        // PCI bus INTA
#define _MB93091_FPGA_IRQ_INTD         (1<<3)        // PCI bus INTA

#define _MB93091_FPGA_GPHL         0xFFC00030      // CB70: GPH and GPL DIP-SW
#define _MB93091_FPGA_CLKRS        0xFFC00104      // For MB93091-CB70, setting of rotary switches
#define _MB93091_FPGA_VDKID	   0xFFC001A0

#endif /* __HAL_MB93091_H__ */
