//==========================================================================
//
//      fr400.h
//
//      HAL misc board support definitions for Fujitsu FR4xx chips
//
//==========================================================================
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

#ifndef __HAL_FR400_H__
#define __HAL_FR400_H__ 1

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
#define _FRV400_LBUS_GCR  0xFE000010		  // General Configuration
#define _FRV400_LBUS_EST  0xFE000020              // Error status
#define _FRV400_LBUS_EAD  0xFE000028              // Error address
#define _FRV400_LBUS_CR0  0xFE000100              // Configuration - space #0
#define _FRV400_LBUS_CR1  0xFE000108              // Configuration - space #1
#define _FRV400_LBUS_CR2  0xFE000110              // Configuration - space #2
#define _FRV400_LBUS_CR3  0xFE000118              // Configuration - space #3
#define _FRV400_LBUS_CR4  0xFE000120              // Configuration - space #4
#define _FRV400_LBUS_CR5  0xFE000128              // Configuration - space #5
#define _FRV400_LBUS_CR6  0xFE000130              // Configuration - space #6
#define _FRV400_LBUS_CR7  0xFE000138              // Configuration - space #7
#define _FRV400_LBUS_BR0  0xFE000C00              // Slave - base address #0
#define _FRV400_LBUS_BR1  0xFE000C08              // Slave - base address #1
#define _FRV400_LBUS_BR2  0xFE000C10              // Slave - base address #2
#define _FRV400_LBUS_BR3  0xFE000C18              // Slave - base address #3
#define _FRV400_LBUS_BR4  0xFE000C20              // Slave - base address #4
#define _FRV400_LBUS_BR5  0xFE000C28              // Slave - base address #5
#define _FRV400_LBUS_BR6  0xFE000C30              // Slave - base address #6
#define _FRV400_LBUS_BR7  0xFE000C38              // Slave - base address #7
#define _FRV400_LBUS_AM0  0xFE000D00              // Slave - address mask #0
#define _FRV400_LBUS_AM1  0xFE000D08              // Slave - address mask #1
#define _FRV400_LBUS_AM2  0xFE000D10              // Slave - address mask #2
#define _FRV400_LBUS_AM3  0xFE000D18              // Slave - address mask #3
#define _FRV400_LBUS_AM4  0xFE000D20              // Slave - address mask #4
#define _FRV400_LBUS_AM5  0xFE000D28              // Slave - address mask #5
#define _FRV400_LBUS_AM6  0xFE000D30              // Slave - address mask #6
#define _FRV400_LBUS_AM7  0xFE000D38              // Slave - address mask #7

// Reset register
#define _FRV400_HW_RESET 0xFEFF0500               // Hardware reset

// Some GPIO magic
#define	_FRV400_GPIO_SIR 0xFEFF0410               // Special input signals
#define	_FRV400_GPIO_SOR 0xFEFF0418               // Special output signals

#endif // __HAL_FR400_H__
