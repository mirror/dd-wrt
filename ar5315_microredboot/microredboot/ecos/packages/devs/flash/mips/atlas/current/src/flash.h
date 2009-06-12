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
// Contributors: gthomas
// Date:         2000-07-26
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef _FLASH_HWR_H_
#define _FLASH_HWR_H_

#define FLASH_Intel_code   0x89

#define FLASH_Read_ID      0x00900090
#define FLASH_Read_Query   0x00980098
#define FLASH_Read_Status  0x00700070
#define FLASH_Clear_Status 0x00500050
#define FLASH_Status_Ready 0x00800080
#define FLASH_Write_Buffer 0x00E800E8
#define FLASH_Program      0x00100010
#define FLASH_Block_Erase  0x00200020
#define FLASH_Set_Lock     0x00600060
#define FLASH_Set_Lock_Confirm 0x00010001
#define FLASH_Clear_Locks  0x00600060
#define FLASH_Clear_Locks_Confirm  0x00D000D0
#define FLASH_Confirm      0x00D000D0
#define FLASH_Configure    0x00B800B8
#define FLASH_Configure_ReadyWait      0x00000000
#define FLASH_Configure_PulseOnErase   0x00010001
#define FLASH_Configure_PulseOnProgram 0x00020002
#define FLASH_Configure_PulseOnBoth    0x00030003
#define FLASH_Reset        0x00FF00FF

#define FLASH_BLOCK_SIZE   0x40000

#define FLASH_Intel_code   0x89

#define FLASH_BASE_MASK 0xff000000

// Extended query information
struct FLASH_query {
    unsigned char manuf_code;
    unsigned char device_code;
    unsigned char _unused0[14];
    unsigned char id[3];  // Q R Y
    unsigned char _unused1[20];
    unsigned char device_size;
    unsigned char device_interface[2];
    unsigned char buffer_size[2];
    unsigned char is_block_oriented;
    unsigned char num_regions[2];
    unsigned char region_size[2];
};

#define ATLAS_SFWCTRL *((volatile unsigned *)0xbf000700)
#define ATLAS_WEN_MAGIC 0xc7

#define FLASH_WRITE_ENABLE()   (ATLAS_SFWCTRL = ATLAS_WEN_MAGIC)
#define FLASH_WRITE_DISABLE()  (ATLAS_SFWCTRL = 0)

#endif  // _FLASH_HWR_H_
