#ifndef CYGONCE_DEVS_FLASH_AMD_AM29XXXXX_INL
#define CYGONCE_DEVS_FLASH_AMD_AM29XXXXX_INL
//==========================================================================
//
//      am29xxxxx.inl
//
//      AMD AM29xxxxx series flash driver
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2004 Gary Thomas
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
// Contributors: gthomas, jskov, Koichi Nagashima
// Date:         2001-02-21
// Purpose:      
// Description:  AMD AM29xxxxx series flash device driver
// Notes:        While the parts support sector locking, some only do so
//               via crufty magic and the use of programmer hardware
//               (specifically by applying 12V to one of the address
//               pins) so the driver does not support write protection.
//
// FIXME:        Should support SW locking on the newer devices.
//
// FIXME:        Figure out how to do proper error checking when there are
//               devices in parallel. Presently the driver will return
//               driver timeout error on device errors which is not very
//               helpful.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/devs_flash_amd_am29xxxxx.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_misc.h>
#include CYGHWR_MEMORY_LAYOUT_H

#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>

//----------------------------------------------------------------------------
// Common device details.
#define FLASH_Read_ID                   FLASHWORD( 0x90 )
#define FLASH_WP_State                  FLASHWORD( 0x90 )
#define FLASH_Reset                     FLASHWORD( 0xF0 )
#define FLASH_Program                   FLASHWORD( 0xA0 )
#define FLASH_Block_Erase               FLASHWORD( 0x30 )
#define FLASH_Load_Buffer               FLASHWORD( 0x25 )
#define FLASH_Flush_Buffer              FLASHWORD( 0x29 )

#define FLASH_Data                      FLASHWORD( 0x80 ) // Data complement
#define FLASH_Busy                      FLASHWORD( 0x40 ) // "Toggle" bit
#define FLASH_Err                       FLASHWORD( 0x20 )
#define FLASH_Sector_Erase_Timer        FLASHWORD( 0x08 )

#define FLASH_unlocked                  FLASHWORD( 0x00 )

#ifndef CYGNUM_FLASH_16AS8
#define _16AS8 0
#else
#define _16AS8 CYGNUM_FLASH_16AS8
#endif

#if (_16AS8 == 0)
# define FLASH_Setup_Addr1              (0x555)
# define FLASH_Setup_Addr2              (0x2AA)
# define FLASH_VendorID_Addr            (0)
# define FLASH_DeviceID_Addr            (1)
# define FLASH_DeviceID_Addr2           (0x0e)
# define FLASH_DeviceID_Addr3           (0x0f)
# define FLASH_WP_Addr                  (2)
#else
# define FLASH_Setup_Addr1              (0xAAA)
# define FLASH_Setup_Addr2              (0x555)
# define FLASH_VendorID_Addr            (0)
# define FLASH_DeviceID_Addr            (2)
# define FLASH_DeviceID_Addr2           (0x1c)
# define FLASH_DeviceID_Addr3           (0x1e)
# define FLASH_WP_Addr                  (4)
#endif
#define FLASH_Setup_Code1               FLASHWORD( 0xAA )
#define FLASH_Setup_Code2               FLASHWORD( 0x55 )
#define FLASH_Setup_Erase               FLASHWORD( 0x80 )

// Platform code must define the below
// #define CYGNUM_FLASH_INTERLEAVE      : Number of interleaved devices (in parallel)
// #define CYGNUM_FLASH_SERIES          : Number of devices in series
// #define CYGNUM_FLASH_WIDTH           : Width of devices on platform
// #define CYGNUM_FLASH_BASE            : Address of first device

// Platform code may define some or all of the below, to provide
// timeouts appropriate to the target hardware. The timeout
// values depend partly on the flash part being used, partly
// on the target (including bus and cpu speeds).
#ifndef CYGNUM_FLASH_TIMEOUT_QUERY
# define CYGNUM_FLASH_TIMEOUT_QUERY		  500000
#endif
#ifndef CYGNUM_FLASH_TIMEOUT_ERASE_TIMER
# define CYGNUM_FLASH_TIMEOUT_ERASE_TIMER	10000000
#endif
#ifndef CYGNUM_FLASH_TIMEOUT_ERASE_COMPLETE
# define CYGNUM_FLASH_TIMEOUT_ERASE_COMPLETE	10000000
#endif
#ifndef CYGNUM_FLASH_TIMEOUT_PROGRAM
# define CYGNUM_FLASH_TIMEOUT_PROGRAM		10000000
#endif

#define CYGNUM_FLASH_BLANK              (1)

#ifndef FLASH_P2V
# define FLASH_P2V( _a_ ) ((volatile flash_data_t *)((CYG_ADDRWORD)(_a_)))
#endif
#ifndef CYGHWR_FLASH_AM29XXXXX_PLF_INIT
# define CYGHWR_FLASH_AM29XXXXX_PLF_INIT()
#endif

//----------------------------------------------------------------------------
// Now that device properties are defined, include magic for defining
// accessor type and constants.
#include <cyg/io/flash_dev.h>

//----------------------------------------------------------------------------
// Information about supported devices
typedef struct flash_dev_info {
    cyg_bool     long_device_id;
    flash_data_t device_id;
    flash_data_t device_id2;
    flash_data_t device_id3;
    cyg_uint32   block_size;
    cyg_int32    block_count;
    cyg_uint32   base_mask;
    cyg_uint32   device_size;
    cyg_bool     bootblock;
    cyg_uint32   bootblocks[64];         // 0 is bootblock offset, 1-11 sub-sector sizes (or 0)
    cyg_bool     banked;
    cyg_uint32   banks[8];               // bank offsets, highest to lowest (lowest should be 0)
                                         // (only one entry for now, increase to support devices
                                         // with more banks).
    cyg_uint32   bufsiz;                 // write buffer size in units of flash_data_t
} flash_dev_info_t;

static const flash_dev_info_t* flash_dev_info;
static const flash_dev_info_t supported_devices[] = {
#include <cyg/io/flash_am29xxxxx_parts.inl>
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

//----------------------------------------------------------------------------
// Auxiliary functions
static volatile flash_data_t * find_bank(volatile flash_data_t * base, void * addr, CYG_ADDRWORD * bo)
    __attribute__ ((section (".2ram.find_bank")));
static flash_data_t * find_sector(volatile flash_data_t * addr, unsigned long *remain_size)
    __attribute__ ((section (".2ram.find_sector")));

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
    volatile flash_data_t *f_s1, *f_s2;
    flash_data_t* id = (flash_data_t*) data;
    flash_data_t w;
    long timeout = CYGNUM_FLASH_TIMEOUT_QUERY;

    ROM = (flash_data_t*) CYGNUM_FLASH_BASE;
    f_s1 = FLASH_P2V(ROM+FLASH_Setup_Addr1);
    f_s2 = FLASH_P2V(ROM+FLASH_Setup_Addr2);

    *f_s1 = FLASH_Reset;
    w = *(FLASH_P2V(ROM));

    *f_s1 = FLASH_Setup_Code1;
    *f_s2 = FLASH_Setup_Code2;
    *f_s1 = FLASH_Read_ID;

    id[0] = -1;
    id[1] = -1;

    // Manufacturers' code
    id[0] = *(FLASH_P2V(ROM+FLASH_VendorID_Addr));
    // Part number
    id[1] = *(FLASH_P2V(ROM+FLASH_DeviceID_Addr));
    id[2] = *(FLASH_P2V(ROM+FLASH_DeviceID_Addr2));
    id[3] = *(FLASH_P2V(ROM+FLASH_DeviceID_Addr3));


    *(FLASH_P2V(ROM)) = FLASH_Reset;

    // Stall, waiting for flash to return to read mode.
    while ((--timeout != 0) && (w != *(FLASH_P2V(ROM)))) ;
}


//----------------------------------------------------------------------------
// Initialize driver details
int
flash_hwr_init(void)
{
    flash_data_t id[4];
    int i;

    CYGHWR_FLASH_AM29XXXXX_PLF_INIT();

    flash_dev_query(id);

    // Look through table for device data
    flash_dev_info = supported_devices;
    for (i = 0; i < NUM_DEVICES; i++) {
        if (!flash_dev_info->long_device_id && flash_dev_info->device_id == id[1])
            break;
        else if ( flash_dev_info->long_device_id && flash_dev_info->device_id == id[1] 
                  && flash_dev_info->device_id2 == id[2] 
                  && flash_dev_info->device_id3 == id[3] )
            break;
        flash_dev_info++;
    }

    // Did we find the device? If not, return error.
    if (NUM_DEVICES == i)
        return FLASH_ERR_DRV_WRONG_PART;

    // Hard wired for now
    flash_info.block_size = flash_dev_info->block_size;
    flash_info.blocks = flash_dev_info->block_count * CYGNUM_FLASH_SERIES;
    flash_info.start = (void *)CYGNUM_FLASH_BASE;
    flash_info.end = (void *)(CYGNUM_FLASH_BASE+ (flash_dev_info->device_size * CYGNUM_FLASH_SERIES));
#ifdef CYGNUM_FLASH_END_RESERVED_BYTES
    flash_info.end = (void *)((unsigned int)flash_info.end - CYGNUM_FLASH_END_RESERVED_BYTES);
#endif
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
// Erase Block

int
flash_erase_block(void* block, unsigned int size)
{
    volatile flash_data_t* ROM, *BANK;
    volatile flash_data_t* b_p = (flash_data_t*) block;
    volatile flash_data_t *b_v;
    volatile flash_data_t *f_s0, *f_s1, *f_s2;
    int timeout = CYGNUM_FLASH_TIMEOUT_QUERY;
    int len = 0;
    int res = FLASH_ERR_OK;
    flash_data_t state;
    cyg_bool bootblock = false;
    cyg_uint32 *bootblocks = (cyg_uint32 *)0;
    CYG_ADDRWORD bank_offset;
    ROM = (volatile flash_data_t*)((unsigned long)block & flash_dev_info->base_mask);
    BANK = find_bank(ROM, block, &bank_offset);

    f_s0 = FLASH_P2V(BANK);
    f_s1 = FLASH_P2V(BANK + FLASH_Setup_Addr1);
    f_s2 = FLASH_P2V(BANK + FLASH_Setup_Addr2);

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
                break;
            } else {
                int ls = flash_dev_info->block_size;
                // Skip over segment
                while ((ls -= *bootblocks++) > 0) ;
            }
        }
    }

    while (size > 0) {
#ifndef CYGHWR_FLASH_AM29XXXXX_NO_WRITE_PROTECT
        // First check whether the block is protected
        *f_s1 = FLASH_Setup_Code1;
        *f_s2 = FLASH_Setup_Code2;
        *f_s1 = FLASH_WP_State;
        state = *FLASH_P2V(b_p+FLASH_WP_Addr);
        *f_s0 = FLASH_Reset;

        if (FLASH_unlocked != state)
            return FLASH_ERR_PROTECT;
#endif

        b_v = FLASH_P2V(b_p);
    
        // Send erase block command - six step sequence
        *f_s1 = FLASH_Setup_Code1;
        *f_s2 = FLASH_Setup_Code2;
        *f_s1 = FLASH_Setup_Erase;
        *f_s1 = FLASH_Setup_Code1;
        *f_s2 = FLASH_Setup_Code2;
        *b_v = FLASH_Block_Erase;

        // Now poll for the completion of the sector erase timer (50us)
        timeout = CYGNUM_FLASH_TIMEOUT_ERASE_TIMER;              // how many retries?
        while (true) {
            state = *b_v;
            if ((state & FLASH_Sector_Erase_Timer)
				== FLASH_Sector_Erase_Timer) break;

            if (--timeout == 0) {
                res = FLASH_ERR_DRV_TIMEOUT;
                break;
            }
        }

        // Then wait for erase completion.
        if (FLASH_ERR_OK == res) {
            timeout = CYGNUM_FLASH_TIMEOUT_ERASE_COMPLETE;
            while (true) {
                state = *b_v;
                if (FLASH_BlankValue == state) {
                    break;
                }

                // Don't check for FLASH_Err here since it will fail
                // with devices in parallel because these may finish
                // at different times.

                if (--timeout == 0) {
                    res = FLASH_ERR_DRV_TIMEOUT;
                    break;
                }
            }
        }

        if (FLASH_ERR_OK != res)
            *FLASH_P2V(ROM) = FLASH_Reset;

        size -= len;  // This much has been erased

        // Verify erase operation
        while (len > 0) {
            b_v = FLASH_P2V(b_p++);
            if (*b_v != FLASH_BlankValue) {
                // Only update return value if erase operation was OK
                if (FLASH_ERR_OK == res) res = FLASH_ERR_DRV_VERIFY;
                return res;
            }
            len -= sizeof(*b_p);
        }

        if (bootblock) {
            len = *bootblocks++;
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
    volatile flash_data_t* BANK;
    volatile flash_data_t* SECT=NULL;
    volatile flash_data_t* data_ptr = (volatile flash_data_t*) data;
    volatile flash_data_t* addr_p = (flash_data_t*) addr;
    volatile flash_data_t* addr_v = FLASH_P2V(addr_p);
    volatile flash_data_t *f_s1, *f_s2;
    CYG_ADDRWORD bank_offset;
    int timeout;
    int res = FLASH_ERR_OK;
    const CYG_ADDRWORD mask  =
        flash_dev_info->bufsiz * sizeof (flash_data_t) - 1;
    unsigned long rem_sect_size;

    // check the address is suitably aligned
    if ((unsigned long)addr & (CYGNUM_FLASH_INTERLEAVE * CYGNUM_FLASH_WIDTH / 8 - 1))
        return FLASH_ERR_INVALID;

    // Base address of device(s) being programmed.
    ROM = (volatile flash_data_t*)((unsigned long)addr_p & flash_dev_info->base_mask);
    BANK = find_bank(ROM, addr, &bank_offset);

    f_s1 = FLASH_P2V(BANK + FLASH_Setup_Addr1);
    f_s2 = FLASH_P2V(BANK + FLASH_Setup_Addr2);
    rem_sect_size = 0;
    len /= sizeof (flash_data_t);

    while (len > 0) {
        flash_data_t state;
        unsigned int nwords;

        addr_v = FLASH_P2V(addr_p);

        if (flash_dev_info->bufsiz > 1) {
            // Assume buffer size is power of two
            unsigned int i;
            
            if (rem_sect_size == 0) {
                SECT = find_sector(addr_v, &rem_sect_size);
                rem_sect_size /= sizeof (flash_data_t);
            }

            // Compute word count to write                
            nwords = flash_dev_info->bufsiz
                - (((CYG_ADDRWORD) addr_v & mask) / sizeof (flash_data_t));
            if (nwords > len)
                nwords = len;

            // Initiate buffered write
            *f_s1 = FLASH_Setup_Code1;
            *f_s2 = FLASH_Setup_Code2;
            *SECT = FLASH_Load_Buffer;
            *SECT = FLASHWORD(nwords - 1);  // All devices need to see this

            // Load data into write buffer, flush buffer
            for(i = 0; i < nwords; i++)
                *addr_v++ = *data_ptr++;
            --addr_v; --data_ptr;
            *SECT = FLASH_Flush_Buffer;
            rem_sect_size -= nwords;
        } else {
            // Program data [byte] - 4 step sequence
            *f_s1 = FLASH_Setup_Code1;
            *f_s2 = FLASH_Setup_Code2;
            *f_s1 = FLASH_Program;
            *addr_v = *data_ptr;
            
            nwords = 1;
        }

        addr_p += nwords;
        timeout = CYGNUM_FLASH_TIMEOUT_PROGRAM;
        while (true) {
            state = *addr_v;
            if (*data_ptr == state) {
                break;
            }

            // Can't check for FLASH_Err since it'll fail in parallel
            // configurations.

            if (--timeout == 0) {
                res = FLASH_ERR_DRV_TIMEOUT;
                break;
            }
        }

        if (FLASH_ERR_OK != res)
            *FLASH_P2V(ROM) = FLASH_Reset;

        if (*addr_v != *data_ptr++) {
            // Only update return value if erase operation was OK
            if (FLASH_ERR_OK == res) res = FLASH_ERR_DRV_VERIFY;
            break;
        }
        len -= nwords;
    }

    // Ideally, we'd want to return not only the failure code, but also
    // the address/device that reported the error.
    return res;
}

static volatile flash_data_t *
find_bank(volatile flash_data_t * base, void * addr, CYG_ADDRWORD * bo)
{
    volatile flash_data_t * res = base;

    if (flash_dev_info->banked) {
        int b = 0;
        *bo = (unsigned long)addr & ~(flash_dev_info->block_size-1);
        *bo -= (unsigned long) base;
        for(;;) {
            if (*bo >= flash_dev_info->banks[b]) {
                res = (volatile flash_data_t*) ((unsigned long)base + flash_dev_info->banks[b]);
                break;
            }
            b++;
        }
    }
    
    return res;
}

static flash_data_t *
find_sector(volatile flash_data_t * addr, unsigned long *remain_size)
{
    const CYG_ADDRESS mask = flash_dev_info->block_size - 1;
    const CYG_ADDRESS a = (CYG_ADDRESS) addr;
    const CYG_ADDRESS base = a & flash_dev_info->base_mask;
    CYG_ADDRESS res = a & ~mask;
    
    *remain_size = flash_dev_info->block_size - (a & mask);

    if (flash_dev_info->bootblock) {
        cyg_uint32 * bootblocks = flash_dev_info->bootblocks;
        while (*bootblocks != _LAST_BOOTBLOCK) {
            int ls = flash_dev_info->block_size;

            if (*bootblocks++ == (res - base)) {
                while (res + *bootblocks < a) {
                    res += *bootblocks++;
                }
            } else {
                // Skip over segment
                while ((ls -= *bootblocks++) > 0) ;
            }
        }

        if (*bootblocks != _LAST_BOOTBLOCK)   
            *remain_size = *bootblocks - (a - res);
    }
    
    return (flash_data_t *) res;
}

#endif // CYGONCE_DEVS_FLASH_AMD_AM29XXXXX_INL
