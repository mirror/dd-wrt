#ifndef CYGONCE_HAL_ARM_XSCALE_MPC50_H
#define CYGONCE_HAL_ARM_XSCALE_MPC50_H
//==========================================================================
//
//      mpc50.h
//
//      Platform specific support (register layout, etc)
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
// Author(s):    <knud.woehler@microplex.de>
// Date:         2003-01-06
//
//####DESCRIPTIONEND####
//
//==========================================================================

// FPGA
// Config
#define MPC50_FPGA_DATA_CLOCK		0x00000004
#define MPC50_FPGA_DATA			0x00000008
#define MPC50_FPGA_CONFIG		0x00000010
#define MPC50_FPGA_STATUS		0x00000020
#define MPC50_FPGA_CONFIG_DONE	0x00000040
// Use
#define MPC50_FPGA_IRQ0			CYGNUM_HAL_INTERRUPT_GPIO0
#define MPC50_FPGA_IRQ1			CYGNUM_HAL_INTERRUPT_GPIO1

#define MPC_FPGA_IRQ			MPC50_FPGA_IRQ0
#define MPC_FPGA_BASE			((unsigned char *)0x60000000)

// Ethernet
#define MPC50_ETH_IOBASE		0x68000000
#define MPC50_ETH_IRQ			CYGNUM_HAL_INTERRUPT_GPIO7
#define MPC50_ETH_RESET			0x00008000

// Flash
#define MPC50_FLASH_BASE		0x50000000

// Button
#ifndef __ASSEMBLER__
#define MPC50_BUTTON_PRESSED		((*PXA2X0_GPLR1 & 0x00000002)==0)
#endif

// static values (offsets)
#define MPC50_VAL_MAGIC			(('5'<<24)+('C'<<16)+('P'<<8)+'M')
#define MPC50_VAL_OFFS_CCCR		4
#define MPC50_VAL_OFFS_MDREFR		8
#define MPC50_VAL_OFFS_MDCNFG		12

#ifndef __ASSEMBLER__
void mpc50_program_new_stack(void *func);
void mpc50_user_hardware_init(void);
#endif




#endif //CYGONCE_HAL_ARM_XSCALE_MPC50_H
