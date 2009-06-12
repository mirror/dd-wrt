#ifndef CYGONCE_DEVS_FLASH_INTEL_SPIFLASH_FLASH_H
#define CYGONCE_DEVS_FLASH_INTEL_SPIFLASH_FLASH_H
//==========================================================================
//
//      spiflash.h
//
//      SPI FLASH programming - device constants, etc.
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
#define CYGNUM_FLASH_BLANK    1
#define CYGNUM_FLASH_DEVICES  1
#define CYGNUM_FLASH_INTERLEAVE CYGNUM_FLASH_DEVICES
#define _FLASH_PRIVATE_
#include <cyg/io/flash_dev.h>
#define flash_t flash_data_t

/*===========================================================================
** !!!!  VERY IMPORTANT NOTICE !!!!  FLASH DATA STORED IN LITTLE ENDIAN FORMAT
**
** This module contains the Serial Flash access routines for the FALCON SOC.
** The FALCON SOC integrates a SPI flash controller that is used to access 
** serial flash parts. The SPI flash controller executes in "Little Endian"
** mode. THEREFORE, all WRITES and READS from the MIPS CPU must be 
** BYTESWAPPED! The SPI Flash controller hardware by default performs READ
** ONLY byteswapping when accessed via the SPI Flash Alias memory region
** (Physical Address 0x0800_0000 - 0x0fff_ffff). The data stored in the 
** flash sectors is stored in "Little Endian" format. 
** 
** The sfiBufferWrite_x32() routine performs byteswapping on all write
** operations.
**===========================================================================*/

#include <cyg/hal/ar2316reg.h>

/*
 * ST Microelectronics Opcodes for Serial Flash
 */

#define STM_OP_WR_ENABLE       0x06     /* Write Enable */
#define STM_OP_WR_DISABLE      0x04     /* Write Disable */
#define STM_OP_RD_STATUS       0x05     /* Read Status */
#define STM_OP_WR_STATUS       0x01     /* Write Status */
#define STM_OP_RD_DATA         0x03     /* Read Data */
#define STM_OP_FAST_RD_DATA    0x0b     /* Fast Read Data */
#define STM_OP_PAGE_PGRM       0x02     /* Page Program */
#define STM_OP_SECTOR_ERASE    0xd8     /* Sector Erase */
#define STM_OP_BULK_ERASE      0xc7     /* Bulk Erase */
#define STM_OP_DEEP_PWRDOWN    0xb9     /* Deep Power-Down Mode */
#define STM_OP_RD_SIG          0xab     /* Read Electronic Signature */

#define STM_STATUS_WIP       0x01       /* Write-In-Progress */
#define STM_STATUS_WEL       0x02       /* Write Enable Latch */
#define STM_STATUS_BP0       0x04       /* Block Protect 0 */
#define STM_STATUS_BP1       0x08       /* Block Protect 1 */
#define STM_STATUS_BP2       0x10       /* Block Protect 2 */
#define STM_STATUS_SRWD      0x80       /* Status Register Write Disable */

#define FLASH0_CS(r) ((r & ~(GPIO_CR_M(GPIO_FLASH0))) | GPIO_CR_M(GPIO_FLASH1))
#define FLASH1_CS(r) ((r & ~(GPIO_CR_M(GPIO_FLASH1))) | GPIO_CR_M(GPIO_FLASH0))
#define FLASH_STANDBY(r) (r | (GPIO_CR_M(GPIO_FLASH0) | GPIO_CR_M(GPIO_FLASH1)))

#define FLASH_1MB  1
#define FLASH_2MB  2
#define FLASH_4MB  3
#define FLASH_8MB  4

#define STM_PAGE_SIZE       256

#define STM_8MBIT_SIGNATURE         0x13
#define STM_M25P80_BYTE_COUNT       1048576
#define STM_M25P80_SECTOR_COUNT     16
#define STM_M25P80_SECTOR_SIZE      0x10000

#define STM_16MBIT_SIGNATURE        0x14
#define STM_M25P16_BYTE_COUNT       2097152
#define STM_M25P16_SECTOR_COUNT     32
#define STM_M25P16_SECTOR_SIZE      0x10000

#define STM_32MBIT_SIGNATURE        0x15
#define STM_M25P32_BYTE_COUNT       4194304
#define STM_M25P32_SECTOR_COUNT     64
#define STM_M25P32_SECTOR_SIZE      0x10000

#define STM_64MBIT_SIGNATURE        0x16
#define STM_M25P64_BYTE_COUNT       8388608
#define STM_M25P64_SECTOR_COUNT     128
#define STM_M25P64_SECTOR_SIZE      0x10000

#define STM_1MB_BYTE_COUNT   STM_M25P80_BYTE_COUNT
#define STM_1MB_SECTOR_COUNT STM_M25P80_SECTOR_COUNT
#define STM_1MB_SECTOR_SIZE  STM_M25P80_SECTOR_SIZE
#define STM_2MB_BYTE_COUNT   STM_M25P16_BYTE_COUNT
#define STM_2MB_SECTOR_COUNT STM_M25P16_SECTOR_COUNT
#define STM_2MB_SECTOR_SIZE  STM_M25P16_SECTOR_SIZE
#define STM_4MB_BYTE_COUNT   STM_M25P32_BYTE_COUNT
#define STM_4MB_SECTOR_COUNT STM_M25P32_SECTOR_COUNT
#define STM_4MB_SECTOR_SIZE  STM_M25P32_SECTOR_SIZE
#define STM_8MB_BYTE_COUNT   STM_M25P64_BYTE_COUNT
#define STM_8MB_SECTOR_COUNT STM_M25P64_SECTOR_COUNT
#define STM_8MB_SECTOR_SIZE  STM_M25P64_SECTOR_SIZE

#ifndef PAGE_SIZE
#define PAGE_SIZE   STM_PAGE_SIZE
#endif

#ifndef DEBUG_PRINT
#define DEBUG_PRINT(args)
#endif

#ifndef cpu2le16
#define  cpu2le16(x)           \
    ((((x) & 0x00ffUL) << 8) | \
    (((x) & 0xff00UL) >> 8))
#endif

#ifndef cpu2le24
#define cpu2le24(x)               \
    ((((x) & 0xff0000UL) >> 16) | \
      ((x) & 0x00ff00UL)        | \
     (((x) & 0x0000ffUL) << 16))
#endif


#ifndef cpu2le32
#define cpu2le32(x)                  \
    ((((x) & 0x000000ffUL) << 24) |  \
     (((x) & 0x0000ff00UL) << 8)  |  \
     (((x) & 0x00ff0000UL) >> 8)  |  \
     (((x) & 0xff000000UL) >> 24))
#endif

#ifndef le2cpu32
#define le2cpu32(x) cpu2le32(x)
#endif


#endif  // CYGONCE_DEVS_FLASH_INTEL_SPIFLASH_FLASH_H
// ------------------------------------------------------------------------
// EOF spiflash.h
