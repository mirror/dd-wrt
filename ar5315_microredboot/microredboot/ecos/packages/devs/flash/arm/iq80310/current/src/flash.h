//==========================================================================
//
//      flash.h
//
//      Flash programming - device constants, etc.
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
// Contributors: gthomas, msalter
// Date:         2000-07-26
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef _FLASH_HWR_H_
#define _FLASH_HWR_H_

// First 4K page of flash at physical address zero is
// virtually mapped at address 0xa0000000.
#define FLASH_P2V(x) ((volatile unsigned char *)(((unsigned)(x) < 0x1000) ?  \
                           ((unsigned)(x) | 0xd0000000) :  \
                           (unsigned)(x)))

#define FLASH_BOOT_BLOCK_SIZE   0x4000

#define FLASH_Intel_code   0x89

#define FLASH_Read_ID      0x90
#define FLASH_Read_Query   0x98
#define FLASH_Read_Status  0x70
#define FLASH_Clear_Status 0x50
#define FLASH_Status_Ready 0x80
#define FLASH_Write_Buffer 0xE8
#define FLASH_Program      0x10
#define FLASH_Block_Erase  0x20
#define FLASH_Set_Lock     0x60
#define FLASH_Set_Lock_Confirm 0x01
#define FLASH_Clear_Locks  0x60
#define FLASH_Clear_Locks_Confirm  0xD0
#define FLASH_Confirm      0xD0
#define FLASH_Configure    0xB8
#define FLASH_Configure_ReadyWait      0x00
#define FLASH_Configure_PulseOnErase   0x01
#define FLASH_Configure_PulseOnProgram 0x02
#define FLASH_Configure_PulseOnBoth    0x03
#define FLASH_Reset        0xFF

#define FLASH_BLOCK_SIZE   0x10000
#define FLASH_WBUF_SIZE    32

#define FLASH_Intel_code   0x89

// Extended query information
struct FLASH_query {
    unsigned char manuf_code;
    unsigned char device_code;
    unsigned char _unused0[14];
    unsigned char id[3];  // Q Q R
    unsigned char _unused1[20];
    unsigned char device_size;
    unsigned char device_interface[2];
    unsigned char buffer_size[2];
    unsigned char is_block_oriented;
    unsigned char num_regions[2];
    unsigned char region_size[2];
};

#endif  // _FLASH_HWR_H_
