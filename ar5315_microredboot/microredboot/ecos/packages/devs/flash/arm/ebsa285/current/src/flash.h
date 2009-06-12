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

// EBSA-285 FLASH layout
//   4x 28F008SA, one device per byte-lane
//   each device is 16x64k blocks

#define FLASH_Read_ID      0x90909090
#define FLASH_Read_Status  0x70707070
#define FLASH_Clear_Status 0x50505050
#define FLASH_Status_Ready 0x80808080
#define FLASH_Program      0x10101010
#define FLASH_Block_Erase  0x20202020
#define FLASH_Confirm      0xD0D0D0D0
#define FLASH_Reset        0xFFFFFFFF

#define FLASH_BLOCK_SIZE   0x40000

#define FLASH_Intel_code   0x89
#define FLASH_28F008SA     0xA2
#define FLASH_28F008SC     0xA6
#endif  // _FLASH_HWR_H_
