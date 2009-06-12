#ifndef CYGONCE_DEVS_FLASH_SST_39VF400_INL
#define CYGONCE_DEVS_FLASH_SST_39VF400_INL
//==========================================================================
//
//      flash_sst_39vf4000.inl
//
//      SST SST39VF400 FLASH driver
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):    Chris Garry <cgarry@sweeneydesign.co.uk>
// Contributors:
// Date:         2003-04-21
// Purpose:
// Description:  SST SST39VF400 flash driver
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/devs_flash_sst_39vf400.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_diag.h> /* HAL_DELAY_US */
#include <cyg/infra/diag.h>   /* Required for diag_printf */
#include CYGHWR_MEMORY_LAYOUT_H

#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>

//----------------------------------------------------------------------------
// Platform code must define the below
// #define CYGNUM_FLASH_INTERLEAVE      : Number of interleaved devices (in parallel)
// #define CYGNUM_FLASH_SERIES          : Number of devices in series
// #define CYGNUM_FLASH_BASE            : Base address of the FLASH
//
// Note:
// Currently the driver only supports CYGNUM_FLASH_INTERLEAVE = 1 and
// CYGNUM_FLASH_SERIES = 1


// Definitions for the SST 39VF400A part
#define SST_ID                    0x00BF          /* SST Manufacturer's ID code   */
#define SST_39VF400A              0x2780          /* SST39VF400/SST39VF400A device code */
#define SST_39VF160               0x2782          /* SST39VF160 device code */

#define CYGNUM_FLASH_SECTOR_SIZE (0x1000)         /* Size of physical sectors */
#define CYGNUM_FLASH_BLOCK_SIZE  (0x1000)         /* Driver 'blocks' may be a multiple of physical sectors */

/*
 * Number of blocks available in flash.
 * For now, hardcode at 2MB flash.
 */
#define CYGNUM_FLASH_BLOCK_NUM   (0x00200000/CYGNUM_FLASH_BLOCK_SIZE)

#define CYGNUM_FLASH_WIDTH       (16)             /* This part is always 16 bits wide */
#define CYGNUM_FLASH_BLANK       (1)


#ifndef FLASH_P2V
# define FLASH_P2V( _a_ ) ((volatile flash_data_t *)((CYG_ADDRWORD)(_a_)))
#endif
#ifndef CYGHWR_FLASH_AM29XXXXX_PLF_INIT
# define CYGHWR_FLASH_AM29XXXXX_PLF_INIT()
#endif

// Structure to hold device ID
typedef struct
{
    cyg_uint16 man_id;
    cyg_uint16 dev_id;
} device_id_t;

// FLASH registers
volatile cyg_uint16 *flash_data_add0 = (cyg_uint16 *)(CYGNUM_FLASH_BASE);
volatile cyg_uint16 *flash_data_add1 = (cyg_uint16 *)(CYGNUM_FLASH_BASE + (0x0001 << 1));
volatile cyg_uint16 *flash_cmd_add1 = (cyg_uint16 *)(CYGNUM_FLASH_BASE + (0x5555 << 1));
volatile cyg_uint16 *flash_cmd_add2 = (cyg_uint16 *)(CYGNUM_FLASH_BASE + (0x2AAA << 1));
volatile cyg_uint16 *flash_cmd_add3 = (cyg_uint16 *)(CYGNUM_FLASH_BASE + (0x5555 << 1));
volatile cyg_uint16 *flash_cmd_add4 = (cyg_uint16 *)(CYGNUM_FLASH_BASE + (0x5555 << 1));
volatile cyg_uint16 *flash_cmd_add5 = (cyg_uint16 *)(CYGNUM_FLASH_BASE + (0x2AAA << 1));


//----------------------------------------------------------------------------
// Now that device properties are defined, include magic for defining
// accessor type and constants.
#include <cyg/io/flash_dev.h>

//----------------------------------------------------------------------------
// Functions that put the flash device into non-read mode must reside
// in RAM.
static device_id_t get_device_id(void) __attribute__ ((section (".2ram.get_device_id")));
int  flash_erase_block(void* block, unsigned int size)
    __attribute__ ((section (".2ram.flash_erase_block")));
int  flash_program_buf(void* addr, void* data, int len)
    __attribute__ ((section (".2ram.flash_program_buf")));

//----------------------------------------------------------------------------
// Get Device ID
//
// Reads the manufacturer and part number codes for the device
//
static device_id_t get_device_id(void)
{
    device_id_t device_id;
    int i;

    /*  Issue the Software ID command */
    *flash_cmd_add1 = 0xAAAA;
    *flash_cmd_add2 = 0x5555;
    *flash_cmd_add3 = 0x9090;

    /* Tida delay time, Tida = 150 ns */
    /* Can use any function that is in ROM */
    for (i = 0; i < 100; i++)
    {
        /* Do nothing */
    }

    /* Read the product ID */
    device_id.man_id = *flash_data_add0 & 0xFF;
    device_id.dev_id = *flash_data_add1;

    /* Issue the Software ID EXIT command */
    *flash_data_add0 = 0xF0F0;

    /* Tida delay time, Tida = 150 ns */
    /* Can use any function that is in ROM */
    for (i = 0; i < 100; i++)
    {
        /* Do nothing */
    }

    return(device_id);
}


//----------------------------------------------------------------------------
// Initialize driver details
//
int flash_hwr_init(void)
{
    device_id_t device_id;

    /* Call the function to get the device ID */
    device_id = get_device_id();

    /* Determine whether there is a known device installed or not */
    if ((device_id.man_id != SST_ID) || 
        ((device_id.dev_id != SST_39VF400A) && (device_id.dev_id != SST_39VF160)))
    {
        return FLASH_ERR_DRV_WRONG_PART;
    }

    // Hard wired for now
    flash_info.block_size = CYGNUM_FLASH_BLOCK_SIZE;
    flash_info.blocks = CYGNUM_FLASH_BLOCK_NUM;
    flash_info.start = (void *)CYGNUM_FLASH_BASE;
    flash_info.end = (void *)(CYGNUM_FLASH_BASE + (flash_info.block_size * flash_info.blocks));
#ifdef CYGNUM_FLASH_END_RESERVED_BYTES
    flash_info.end = (void *)((unsigned int)flash_info.end - CYGNUM_FLASH_END_RESERVED_BYTES);
#endif

    return FLASH_ERR_OK;
}

//----------------------------------------------------------------------------
// Map a hardware status to a package error
int flash_hwr_map_error(int e)
{
    return e;
}


//----------------------------------------------------------------------------
// See if a range of FLASH addresses overlaps currently running code
bool flash_code_overlaps(void *start, void *end)
{
    extern unsigned char _stext[], _etext[];

    return ((((unsigned long)&_stext >= (unsigned long)start) &&
             ((unsigned long)&_stext < (unsigned long)end)) ||
            (((unsigned long)&_etext >= (unsigned long)start) &&
             ((unsigned long)&_etext < (unsigned long)end)));
}

//----------------------------------------------------------------------------
// Erase Block
// This function actually uses the sector erase command instead of the block
// erase command. this allows for the effective block size to be smaller
// than 64K (as small as the 4K sector size)
int flash_erase_block(void* block, unsigned int size)
{

    volatile cyg_uint16 *block_addr;
    volatile cyg_uint16 *verify_addr;
    int verify_failed;
    cyg_uint32 i, timeout;
    int j;
    int retry;

    block_addr = block;

    for (j = 0; j < (size / CYGNUM_FLASH_SECTOR_SIZE); j++)
    {
        retry = 0;
        while (retry < 16)
        {
            /* Issue the Sector-Erase command */
            *flash_cmd_add1 = 0xAAAA;
            *flash_cmd_add2 = 0x5555;
            *flash_cmd_add3 = 0x8080;
            *flash_cmd_add4 = 0xAAAA;
            *flash_cmd_add5 = 0x5555;
            *block_addr = 0x3030;  /* Sector Erase command */

            /* Wait for the Erase operation to complete */
            /* With a timeout to stop the board locking up with a H/W error*/
            timeout = 0;
            i = 0;
            while (i < 5)
            {
                if (*block_addr == 0xFFFF)
                {
                    i++;
                }
                else
                {
                    i = 0;
                }

                if (++timeout > 0x01000000)
                {
                    /* Timeout - return with ERROR status */
                    return (FLASH_ERR_DRV_TIMEOUT);
                }
            }

            /* Verify this sector has been erased */
            verify_addr = block_addr;
            verify_failed = 0;
            while ((cyg_uint32)(verify_addr) < ((cyg_uint32)block_addr + (cyg_uint32)CYGNUM_FLASH_SECTOR_SIZE))
            {
                if (*verify_addr != 0xFFFF)
                {
                    /* Error verifying segment data */
                    retry++;
                    verify_failed = 1;
                    break;
                }
                ++verify_addr;
            }

            if (verify_failed == 0)
            {
                /* Sector erase verified */
                break;
            }
        }

        /* Increment the block address by 1 sector */
        (cyg_uint32)block_addr += CYGNUM_FLASH_SECTOR_SIZE;
    }

    /* Verify the entire block has been set to all 0xFFFFs */
    block_addr = block;
    while ((cyg_uint32)(block_addr) < ((cyg_uint32)block + (cyg_uint32)size))
    {
        if (*block_addr != 0xFFFF)
        {
            /* Error verifying data */
            return (FLASH_ERR_DRV_VERIFY);
        }

        ++block_addr;
    }

    return (FLASH_ERR_OK);
}


//----------------------------------------------------------------------------
// Program Buffer
int
flash_program_buf(void* addr, void* data, int len)
{
    volatile cyg_uint16 *write_ptr;
    volatile cyg_uint16 *data_ptr;
    int i;
    cyg_uint32 timeout;
    cyg_uint16 read_data1, read_data2;

    write_ptr = addr;  /* Initialise local pointers */
    data_ptr = data;

    /* Loop for writing the data */
    while ((cyg_uint32)write_ptr < ((cyg_uint32)addr + (cyg_uint32)len))
    {
        /* Write word of data to FLASH */
        /* Issue the Word-Program command */
        *flash_cmd_add1 = 0xAAAA;
        *flash_cmd_add2 = 0x5555;
        *flash_cmd_add3 = 0xA0A0;
        /* Write data word to FLASH */
        *write_ptr = *data_ptr;

        /* Wait for the operation to complete */
        /* Wait for the Erase operation to complete */
        /* With a timeout to stop the board locking up with a H/W error*/
        timeout = 0;
        i = 0;
        while (i < 5)
        {
            read_data1 = *write_ptr;
            read_data2 = *write_ptr;
            if (read_data1 == read_data2)
            {
                /* Bit 6 can no longer be toggling */
                i++;
            }
            else
            {
                i = 0;
            }

            if (++timeout > 0x01000000)
            {
                /* Timeout - return with ERROR status */
                return (FLASH_ERR_DRV_TIMEOUT);
            }
        }

        /* Increment pointers to next words */
        ++write_ptr;
        ++data_ptr;
    }

    /* Data write complete - verify the data */
    write_ptr = addr;  /* Re-initialise local pointers */
    data_ptr = data;

    /* Loop for verifying the data */
    while ((cyg_uint32)write_ptr < ((cyg_uint32)addr + (cyg_uint32)len))
    {
        if (*write_ptr != *data_ptr)
        {
            /* Error verifying data */
            return (FLASH_ERR_DRV_VERIFY);
        }

        /* Increment pointers to next words */
        ++write_ptr;
        ++data_ptr;
    }

    return (FLASH_ERR_OK);
}

#endif // CYGONCE_DEVS_FLASH_SST_39VF400_INL







