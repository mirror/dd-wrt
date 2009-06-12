#ifndef CYGONCE_DEVS_FLASH_ATMEL_AT49XXXX_INL
#define CYGONCE_DEVS_FLASH_ATMEL_AT49XXXX_INL
//==========================================================================
//
//      at49xxxx.inl
//
//      Atmel AT49xxxx series flash driver
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Jonathan Larmour
// Copyright (C) 2003 Gary Thomas
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
// Author(s):    Jani Monoses <jani@iv.ro>
// Contributors: Cristian Vlasin <cris@iv.ro>, tdrury, jlarmour
// Date:         2002-06-24
// Purpose:
// Description:
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/devs_flash_atmel_at49xxxx.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_diag.h>
#include CYGHWR_MEMORY_LAYOUT_H

#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>


//----------------------------------------------------------------------------
// Common device details.
#define FLASH_Read_ID                   FLASHWORD( 0x90 )
#define FLASH_Read_ID_Exit              FLASHWORD( 0xF0 )
#define FLASH_Program                   FLASHWORD( 0xA0 )
#define FLASH_Sector_Erase              FLASHWORD( 0x30 )
#define FLASH_Chip_Erase                FLASHWORD( 0x10 )

#define FLASH_Busy                      FLASHWORD( 0x40 ) // "Toggle" bit, I/O 6
#define FLASH_InverseData               FLASHWORD( 0x80 ) // I/O 7, Inverse data

#define FLASH_Setup_Addr1               (0x5555)
#define FLASH_Setup_Addr2               (0x2AAA)
#define FLASH_Setup_Code1               FLASHWORD( 0xAA )
#define FLASH_Setup_Code2               FLASHWORD( 0x55 )
#define FLASH_Setup_Erase               FLASHWORD( 0x80 )

#define CYGNUM_FLASH_BLANK             (1)

#ifndef CYGNUM_FLASH_ID_MANUFACTURER
# define CYGNUM_FLASH_ID_MANUFACTURER  FLASHWORD(0x1F)
#endif

//----------------------------------------------------------------------------
// Now that device properties are defined, include magic for defining
// accessor type and constants.
#include <cyg/io/flash_dev.h>

//----------------------------------------------------------------------------
// Information about supported devices
typedef struct flash_dev_info {
    flash_data_t device_id;
#ifdef CYG_FLASH_LONG_DEVICE_NEEDED
    cyg_bool     long_device_id;
    flash_data_t device_id2;
    flash_data_t device_id3;
#endif
    cyg_uint32   block_size;
    cyg_int32    block_count;
    cyg_uint32   base_mask;
    cyg_uint32   device_size;
    cyg_bool     bootblock;
    cyg_bool     chip_erase;
    cyg_uint32   bootblocks[64];         // 0 is bootblock offset, 1-11 sub-sector sizes (or 0)
#ifdef NOTYET // FIXME: not supported yet (use am29xxxxx for template)
    cyg_bool     banked;
    cyg_uint32   banks[8];               // bank offsets, highest to lowest (lowest should be 0)
                                         // (only one entry for now, increase to support devices
                                         // with more banks).
#endif
} flash_dev_info_t;

static const flash_dev_info_t* flash_dev_info;
static const flash_dev_info_t supported_devices[] = {
#include <cyg/io/flash_at49xxxx_parts.inl>
};
#define NUM_DEVICES (sizeof(supported_devices)/sizeof(flash_dev_info_t))

//----------------------------------------------------------------------------
// Functions that put the flash device into non-read mode must reside
// in RAM.
void flash_query(void* data) __attribute__ ((section (".2ram.flash_query")));
int  flash_erase_block(void* block, unsigned int size)
    __attribute__ ((section (".2ram.flash_erase_block")));
int  flash_program_buf(void* addr, void* data, int len)
    __attribute__ ((section (".2ram.flash_program_buf")));
static int wait_while_busy(int timeout, volatile flash_data_t* addr_ptr, flash_data_t value)
    __attribute__ ((section (".2ram.text")));

//----------------------------------------------------------------------------
// Initialize driver details
int
flash_hwr_init(void)
{
    flash_data_t id[4];
    int i;

#ifdef CYGHWR_FLASH_AT49XXXX_PLF_INIT
    CYGHWR_FLASH_AT49XXXX_PLF_INIT();
#endif

    flash_dev_query(id);

    // Check that flash_id data is matching the one the driver was
    // configured for.

    // Check manufacturer
    if (id[0] != CYGNUM_FLASH_ID_MANUFACTURER)
        return FLASH_ERR_DRV_WRONG_PART;

    // Look through table for device data
    flash_dev_info = supported_devices;
#ifdef CYG_FLASH_LONG_DEVICE_NEEDED
    for (i = 0; i < NUM_DEVICES; i++) {
        if (!flash_dev_info->long_device_id && flash_dev_info->device_id == id[1])
            break;
        else if ( flash_dev_info->long_device_id && flash_dev_info->device_id == id[1] 
                  && flash_dev_info->device_id2 == id[2] 
                  && flash_dev_info->device_id3 == id[3] )
            break;
        flash_dev_info++;
    }
#else
    for (i = 0; i < NUM_DEVICES; i++) {
        if (flash_dev_info->device_id == id[1])
            break;
        flash_dev_info++;
    }
#endif

    // Did we find the device? If not, return error.
    if (NUM_DEVICES == i)
        return FLASH_ERR_DRV_WRONG_PART;

    // Hard wired for now
    flash_info.block_size = flash_dev_info->block_size;
    flash_info.blocks = flash_dev_info->block_count * CYGNUM_FLASH_SERIES;
    flash_info.start = (void *)CYGNUM_FLASH_BASE;
    flash_info.end = (void *)(CYGNUM_FLASH_BASE+ (flash_dev_info->device_size * CYGNUM_FLASH_SERIES));
    return FLASH_ERR_OK;
}

//----------------------------------------------------------------------------
// Map a hardware status to a package error
int
flash_hwr_map_error(int e)
{
    return e;
}


//----------------------------------------------------------------------------
// See if a range of FLASH addresses overlaps currently running code
bool
flash_code_overlaps(void *start, void *end)
{
    extern unsigned char _stext[], _etext[];

    return ((((unsigned long)&_stext >= (unsigned long)start) &&
             ((unsigned long)&_stext < (unsigned long)end)) ||
            (((unsigned long)&_etext >= (unsigned long)start) &&
             ((unsigned long)&_etext < (unsigned long)end)));
}

//----------------------------------------------------------------------------
// Flash Query
//
// Only reads the manufacturer and part number codes for the first
// device(s) in series. It is assumed that any devices in series
// will be of the same type.

void
flash_query(void* data)
{
    volatile flash_data_t *ROM;
    flash_data_t* id = (flash_data_t*) data;

    ROM = (volatile flash_data_t*) CYGNUM_FLASH_BASE;

    ROM[FLASH_Setup_Addr1] = FLASH_Setup_Code1;
    ROM[FLASH_Setup_Addr2] = FLASH_Setup_Code2;
    ROM[FLASH_Setup_Addr1] = FLASH_Read_ID;

    // FIXME: 10ms delay?

    // Manufacturers' code
    id[0] = ROM[0];
    // Part number
    id[1] = ROM[1];

    ROM[FLASH_Setup_Addr1] = FLASH_Setup_Code1;
    ROM[FLASH_Setup_Addr2] = FLASH_Setup_Code2;
    ROM[FLASH_Setup_Addr1] = FLASH_Read_ID_Exit;

    // FIXME: 10ms delay?
}

// Wait for completion. While programming/erasing check
// that i/o 7 is inverse of data as described in Atmels examples.

static int wait_while_busy(int timeout, volatile flash_data_t* addr_ptr, flash_data_t expected)
{
	int val;
	flash_data_t state;
        while (true) {
            state = *addr_ptr & FLASH_InverseData;
            if (state==(expected&FLASH_InverseData)) {
            	val=FLASH_ERR_OK;
            	break;
	    }
            if (--timeout == 0) {
                val=FLASH_ERR_DRV_TIMEOUT;
                break;
            }
        }
        return val;
}

//----------------------------------------------------------------------------
// Erase Block
int
flash_erase_block(void* block, unsigned int size)
{
    volatile flash_data_t* ROM;
    volatile flash_data_t* b_p = (volatile flash_data_t*) block;

    int res = FLASH_ERR_OK;
    unsigned int len = 0;
    cyg_bool bootblock = false;
    cyg_uint32 *bootblocks = (cyg_uint32 *)0;

    //	diag_printf("\nERASE: Block %p, size: %u\n",block,size);

    // Base address of device(s) being programmed.
    ROM = (volatile flash_data_t*) ((unsigned long)block & flash_dev_info->base_mask);


#if defined(CYGHWR_DEVS_FLASH_ATMEL_AT49XXXX_ERASE_BUG_WORKAROUND)

    // Before erasing the data, overwrite it with all zeroes. This is a workaround
    // for a silicon bug that affects erasing of some devices, see
    // http://www.atmel.com/dyn/resources/prod_documents/doc6076.pdf.
    for (len = size / sizeof *b_p; (FLASH_ERR_OK == res) && (len > 0); len--, b_p++) {
        // Program data [byte] - 4 step sequence
        ROM[FLASH_Setup_Addr1] = FLASH_Setup_Code1;
        ROM[FLASH_Setup_Addr2] = FLASH_Setup_Code2;
        ROM[FLASH_Setup_Addr1] = FLASH_Program;
        *b_p = 0;
                
        res = wait_while_busy(5000000, b_p, 0);

        if (*b_p != 0)
            // Only update return value if operation was OK
            if (FLASH_ERR_OK == res) res = FLASH_ERR_DRV_VERIFY;
    }
    
    if (FLASH_ERR_OK != res)
        return res;
    
    b_p = (volatile flash_data_t*) block;

#endif // defined(CYGHWR_DEVS_FLASH_ATMEL_AT49XXXX_ERASE_BUG_WORKAROUND)

    // Assume not "boot" sector, full size
    bootblock = false;
    len = flash_dev_info->block_size;
        
    // Is this in a "boot" sector?
    if (flash_dev_info->bootblock) {
        bootblocks = (cyg_uint32 *)&flash_dev_info->bootblocks[0];
        while (*bootblocks != _LAST_BOOTBLOCK) {
            if (*bootblocks++ == ((unsigned long)block - (unsigned long)ROM)) {
                len = *bootblocks++;  // Size of first sub-block
                bootblock = true;
                //	diag_printf("\nERASE: Is Boot block - size: %d, ptr %p\n",len,b_p);
                break;
            } else {
                int ls = flash_dev_info->block_size;
                // Skip over segment
                while ((ls -= *bootblocks++) > 0) ;
            }
        }
    }

    while (size > 0) {
        //Erase sector 6-byte sequence
        ROM[FLASH_Setup_Addr1] = FLASH_Setup_Code1;
        ROM[FLASH_Setup_Addr2] = FLASH_Setup_Code2;
        ROM[FLASH_Setup_Addr1] = FLASH_Setup_Erase;
        ROM[FLASH_Setup_Addr1] = FLASH_Setup_Code1;
        ROM[FLASH_Setup_Addr2] = FLASH_Setup_Code2;
        if (flash_dev_info->chip_erase) {
            // Can only erase the entire device!
            if (b_p == ROM) {
                ROM[FLASH_Setup_Addr1] = FLASH_Chip_Erase;
            } else {
                res = FLASH_ERR_DRV_VERIFY;
            }
        } else {
            *b_p = FLASH_Sector_Erase;
        }

        size -= len;  // This much has been erased

	res = wait_while_busy(66000000, b_p, FLASH_BlankValue);

        // Verify erase operation
        if (FLASH_ERR_OK == res) {
            while (len > 0) {
                if (*b_p != FLASH_BlankValue) {
                    break;
                }
                len -= sizeof(*b_p);
                b_p++;
            }                    
        }
                    
        if (FLASH_ERR_OK != res)
            break;

        if (bootblock) {
            len = *bootblocks++;
            //	diag_printf("\nERASE: Is Boot block - size: %d, len %d, ptr %p\n",size,len,b_p);
        }
    }
    return res;
}

//----------------------------------------------------------------------------
// Program Buffer
int
flash_program_buf(void* addr, void* data, int len)
{
    volatile flash_data_t* ROM;
    volatile flash_data_t* addr_ptr = (volatile flash_data_t*) addr;
    volatile flash_data_t* data_ptr = (volatile flash_data_t*) data;
    int res = FLASH_ERR_OK;
	
    // check the address is suitably aligned
    if ((unsigned long)addr & (CYGNUM_FLASH_INTERLEAVE * CYGNUM_FLASH_WIDTH / 8 - 1))
        return FLASH_ERR_INVALID;

    // Base address of device(s) being programmed. 
    ROM = (volatile flash_data_t*)((unsigned long)addr_ptr & flash_dev_info->base_mask);

    while ((FLASH_ERR_OK == res) && (len > 0)) {
        // Program data [byte] - 4 step sequence
        ROM[FLASH_Setup_Addr1] = FLASH_Setup_Code1;
        ROM[FLASH_Setup_Addr2] = FLASH_Setup_Code2;
        ROM[FLASH_Setup_Addr1] = FLASH_Program;
        addr_ptr[0] = data_ptr[0];
                
        res = wait_while_busy(5000000,addr_ptr, data_ptr[0]);

        if (*addr_ptr++ != *data_ptr++) {
            // Only update return value if operation was OK
            if (FLASH_ERR_OK == res) res = FLASH_ERR_DRV_VERIFY;
            break;
        }

        len -= sizeof (*data_ptr);
    }

    return res;
}

#endif // CYGONCE_DEVS_FLASH_ATMEL_AT49XXXX_INL

// EOF flash_at49xxxx.inl
