//==========================================================================
//
//      fr500.h
//
//      HAL misc board support definitions for Fujitsu FR5xx chips
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

#ifndef __HAL_FR500_H__
#define __HAL_FR500_H__ 1


// SDRAM Controller
#define _FRV550_SDRAM_CTL  0xFEFF0200              // Control
#define _FRV550_SDRAM_AMC  0xFEFF0204              // Access mode control
#define _FRV550_SDRAM_MS   0xFEFF0208              // Mode select
#define _FRV550_SDRAM_CFG  0xFEFF020C              // Configuration
#define _FRV550_SDRAM_AN   0xFEFF0210              // Address number
#define _FRV550_SDRAM_STS  0xFEFF0214              // Status
#define _FRV550_SDRAM_RCN  0xFEFF0218              // Refresh control
#define _FRV550_SDRAM_ART  0xFEFF021C              // Auto-refresh timer

#define _FRV550_SDRAM_ARS0 0xFEFF0100              // Address #0
#define _FRV550_SDRAM_ARS1 0xFEFF0104              // Address #1
#define _FRV550_SDRAM_ARS2 0xFEFF0108              // Address #2
#define _FRV550_SDRAM_ARS3 0xFEFF010C              // Address #3
#define _FRV550_SDRAM_AMK0 0xFEFF0110              // Address mask #0
#define _FRV550_SDRAM_AMK1 0xFEFF0114              // Address mask #1
#define _FRV550_SDRAM_AMK2 0xFEFF0118              // Address mask #2
#define _FRV550_SDRAM_AMK3 0xFEFF011C              // Address mask #3

// Local bus control
#define _FRV550_LBUS_CP    0xFEFF1000              // Controller protect
#define _FRV550_LBUS_GCR   0xFEFF1010              // General configuration
#define _FRV550_LBUS_EST   0xFEFF1020              // Error status
#define _FRV550_LBUS_EAD   0xFEFF1028              // Error address
#define _FRV550_LBUS_MAICR 0xFEFF1030              // Master access interval control
#define _FRV550_LBUS_EMBR  0xFEFF1040              // External master base
#define _FRV550_LBUS_EMAM  0xFEFF1048              // External master address mask
#define _FRV550_LBUS_CR0   0xFEFF1100              // Configuration - space #0
#define _FRV550_LBUS_CR1   0xFEFF1108              // Configuration - space #1
#define _FRV550_LBUS_CR2   0xFEFF1110              // Configuration - space #2
#define _FRV550_LBUS_CR3   0xFEFF1118              // Configuration - space #3
#define _FRV550_LBUS_CR4   0xFEFF1120              // Configuration - space #4
#define _FRV550_LBUS_CR5   0xFEFF1128              // Configuration - space #5
#define _FRV550_LBUS_CR6   0xFEFF1130              // Configuration - space #6
#define _FRV550_LBUS_CR7   0xFEFF1138              // Configuration - space #7
#define _FRV550_LBUS_BR0   0xFEFF1C00              // Slave - base address #0
#define _FRV550_LBUS_BR1   0xFEFF1C08              // Slave - base address #1
#define _FRV550_LBUS_BR2   0xFEFF1C10              // Slave - base address #2
#define _FRV550_LBUS_BR3   0xFEFF1C18              // Slave - base address #3
#define _FRV550_LBUS_BR4   0xFEFF1C20              // Slave - base address #4
#define _FRV550_LBUS_BR5   0xFEFF1C28              // Slave - base address #5
#define _FRV550_LBUS_BR6   0xFEFF1C30              // Slave - base address #6
#define _FRV550_LBUS_BR7   0xFEFF1C38              // Slave - base address #7
#define _FRV550_LBUS_AM0   0xFEFF1D00              // Slave - address mask #0
#define _FRV550_LBUS_AM1   0xFEFF1D08              // Slave - address mask #1
#define _FRV550_LBUS_AM2   0xFEFF1D10              // Slave - address mask #2
#define _FRV550_LBUS_AM3   0xFEFF1D18              // Slave - address mask #3
#define _FRV550_LBUS_AM4   0xFEFF1D20              // Slave - address mask #4
#define _FRV550_LBUS_AM5   0xFEFF1D28              // Slave - address mask #5
#define _FRV550_LBUS_AM6   0xFEFF1D30              // Slave - address mask #6
#define _FRV550_LBUS_AM7   0xFEFF1D38              // Slave - address mask #7

// Reset register
#define _FRV550_HW_RESET 0xFEFFF500               // Hardware reset

// Some GPIO magic
#define _FRV550_GPIO_SIR 0xFEFFF410               // Special input signals
#define _FRV550_GPIO_SOR 0xFEFFF418               // Special output signals

#endif // __HAL_FR500_H__
