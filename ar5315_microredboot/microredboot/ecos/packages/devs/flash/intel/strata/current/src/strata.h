#ifndef CYGONCE_DEVS_FLASH_INTEL_STRATA_FLASH_H
#define CYGONCE_DEVS_FLASH_INTEL_STRATA_FLASH_H
//==========================================================================
//
//      strata.h
//
//      strataFlash programming - device constants, etc.
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
// Author(s):    gthomas, hmt
// Contributors: gthomas
// Date:         2001-02-14
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/devs_flash_strata.h>
#include CYGDAT_DEVS_FLASH_STRATA_INL

// ------------------------------------------------------------------------
// 
// It is expected that the above include defined all the properties of the
// device we want to drive: the choices this module supports include:
//
//                                    Buffered  Read     Block
//                                     write    query    locking
// 28FxxxB3 - Bootblock               - no      no       no
// 28FxxxC3 - StrataFlash             - no      yes      yes
// 28FxxxJ3 - Advanced StrataFlash    - yes     yes      yes
// 28FxxxK3 - Synchronous StrataFlash - yes     yes      yes
// 
// These options are controlled by defining or not, in that include file,
// these symbols (not CDL options, just symbols - though they could be CDL
// in future)
//         CYGOPT_FLASH_IS_BOOTBLOCK     - for xxxB3 devices.
//         CYGOPT_FLASH_IS_NOT_ADVANCED  - for xxxC3 devices.
//         CYGOPT_FLASH_IS_SYNCHRONOUS   - for xxxK3 devices.
//         none of the above             - for xxxJ3 devices.  
// (Advanced seems to be usual these days hence the sense of that opt)
//
// Other properties are controlled by these symbols:
//         CYGNUM_FLASH_DEVICES 	number of devices across the databus
//         CYGNUM_FLASH_WIDTH 	        number of bits in each device
//         CYGNUM_FLASH_BLANK           1 if blank is allones, 0 if 0
//         CYGNUM_FLASH_BASE 	        base address
//         CYGNUM_FLASH_BASE_MASK       a mask to get base address from any
// 
// for example, a 32-bit memory could be made from 1x32bit, 2x16bit or
// 4x8bit devices; usually 16bit ones are chosen in practice, so we would
// have CYGNUM_FLASH_DEVICES = 2, and CYGNUM_FLASH_WIDTH = 16.  Both
// devices would be handled simulataneously, via 32bit bus operations.
// Some CPUs can handle a single 16bit device as 32bit memory "by magic".
// In that case, CYGNUM_FLASH_DEVICES = 1 and CYGNUM_FLASH_WIDTH = 16, and
// the device is managed using only 16bit bus operations.

#define CYGNUM_FLASH_INTERLEAVE CYGNUM_FLASH_DEVICES
#define _FLASH_PRIVATE_
#include <cyg/io/flash_dev.h>

#define flash_t flash_data_t
// ------------------------------------------------------------------------
//
// This generic code is intended to deal with all shapes and orientations
// of Intel StrataFlash.  Trademarks &c belong to their respective owners.
//
// It therefore needs some trickery to define the constants and accessor
// types that we use to interact with the device or devices.
//
// The assumptions are that
//  o Parallel devices, we write to, with the "opcode" replicated per
//    device
//  o The "opcode" and status returns exist only in the low byte of the
//    device's interface regardless of its width.
//  o Hence opcodes and status are only one byte.
// An exception is the test for succesfully erased data.
//
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

#define FLASH_Read_ID      		FLASHWORD( 0x90 )
#ifndef CYGOPT_FLASH_IS_BOOTBLOCK
#define FLASH_Read_Query   		FLASHWORD( 0x98 ) // Strata only
#endif
#define FLASH_Read_Status  		FLASHWORD( 0x70 )
#define FLASH_Clear_Status 		FLASHWORD( 0x50 )
#define FLASH_Status_Ready 		FLASHWORD( 0x80 )
#ifdef CYGOPT_FLASH_IS_BOOTBLOCK
#define FLASH_Program      		FLASHWORD( 0x40 ) // BootBlock only
#else
#define FLASH_Program      		FLASHWORD( 0x10 )
#endif
#define FLASH_Block_Erase  		FLASHWORD( 0x20 )
#ifndef CYGOPT_FLASH_IS_BOOTBLOCK
#ifndef CYGOPT_FLASH_IS_NOT_ADVANCED
#define FLASH_Write_Buffer 		FLASHWORD( 0xE8 ) // *Advanced* Strata only
#endif // flash is advanced ie. has Write Buffer command
#define FLASH_Set_Lock     		FLASHWORD( 0x60 ) // Strata only
#define FLASH_Set_Lock_Confirm 		FLASHWORD( 0x01 ) // Strata only
#define FLASH_Clear_Locks  		FLASHWORD( 0x60 ) // Strata only
#define FLASH_Clear_Locks_Confirm	FLASHWORD( 0xD0 ) // Strata only
#endif
#define FLASH_Confirm      		FLASHWORD( 0xD0 )
//#define FLASH_Configure    			FLASHWORD( 0xB8 )
//#define FLASH_Configure_ReadyWait      	FLASHWORD( 0x00 )
//#define FLASH_Configure_PulseOnErase   	FLASHWORD( 0x01 )
//#define FLASH_Configure_PulseOnProgram 	FLASHWORD( 0x02 )
//#define FLASH_Configure_PulseOnBoth    	FLASHWORD( 0x03 )
#define FLASH_Reset        		FLASHWORD( 0xFF )
                                                     
// Status that we read back:                         
#define FLASH_ErrorMask			FLASHWORD( 0x7E )
#define FLASH_ErrorProgram		FLASHWORD( 0x10 )
#define FLASH_ErrorErase		FLASHWORD( 0x20 )

#define FLASH_ErrorNotVerified		FLASHWORD( 0x9910 ) // made-up number

// ------------------------------------------------------------------------

#define FLASH_Intel_code   0x89 // NOT mapped to 16+16
#define FLASH_STMicro_code 0x20 // NOT mapped to 16+16

// Extended query information
struct FLASH_query {
    unsigned char manuf_code;    // FLASH_Intel_code
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

#endif  // CYGONCE_DEVS_FLASH_INTEL_STRATA_FLASH_H
// ------------------------------------------------------------------------
// EOF strata.h
