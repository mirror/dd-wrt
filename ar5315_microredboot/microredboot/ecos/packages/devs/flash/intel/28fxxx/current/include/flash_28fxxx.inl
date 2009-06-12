#ifndef CYGONCE_DEVS_FLASH_INTEL_28FXXX_INL
#define CYGONCE_DEVS_FLASH_INTEL_28FXXX_INL
//==========================================================================
//
//      flash_28fxxx.inl
//
//      Intel 28Fxxx series flash driver
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
// Author(s):    jskov
// Contributors: jskov
// Date:         2001-03-21
// Purpose:      
// Description:  
//              
// Notes:        Device table could use unions of flags to save some space
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_flash.h>
#include <pkgconf/devs_flash_intel_28fxxx.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/hal/hal_io.h>

#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>

#define nDEBUG

#ifdef DEBUG
typedef void (*call_t)(char* str, ...);
extern void diag_printf(char* str, ...);
call_t d_print = &diag_printf;
#endif

//----------------------------------------------------------------------------
// Common device details.
#define FLASH_Read_ID                   FLASHWORD( 0x90 )
#define FLASH_Reset                     FLASHWORD( 0xFF )
#define FLASH_Program                   FLASHWORD( 0x40 )
#define FLASH_Write_Buffer              FLASHWORD( 0xe8 )
#define FLASH_Block_Erase               FLASHWORD( 0x20 )
#define FLASH_Confirm                   FLASHWORD( 0xD0 )
#define FLASH_Resume                    FLASHWORD( 0xD0 )

#define FLASH_Set_Lock                  FLASHWORD( 0x60 )
#define FLASH_Set_Lock_Confirm          FLASHWORD( 0x01 )
#define FLASH_Clear_Lock                FLASHWORD( 0x60 )
#define FLASH_Clear_Lock_Confirm        FLASHWORD( 0xd0 )

#define FLASH_Read_Status               FLASHWORD( 0x70 )
#define FLASH_Clear_Status              FLASHWORD( 0x50 )
#define FLASH_Status_Ready              FLASHWORD( 0x80 )

// Status that we read back:                         
#define FLASH_ErrorMask                 FLASHWORD( 0x7E )
#define FLASH_ErrorProgram              FLASHWORD( 0x10 )
#define FLASH_ErrorErase                FLASHWORD( 0x20 )
#define FLASH_ErrorLock                 FLASHWORD( 0x30 )
#define FLASH_ErrorLowVoltage           FLASHWORD( 0x08 )
#define FLASH_ErrorLocked               FLASHWORD( 0x02 )

// Platform code must define the below
// #define CYGNUM_FLASH_INTERLEAVE      : Number of interleaved devices (in parallel)
// #define CYGNUM_FLASH_SERIES          : Number of devices in series
// #define CYGNUM_FLASH_WIDTH           : Width of devices on platform
// #define CYGNUM_FLASH_BASE            : Address of first device

#define CYGNUM_FLASH_BLANK              (1)
#define CYGNUM_FLASH_DEVICES            (CYGNUM_FLASH_INTERLEAVE*CYGNUM_FLASH_SERIES)


#ifndef FLASH_P2V
# define FLASH_P2V( _a_ ) ((volatile flash_data_t *)((CYG_ADDRWORD)(_a_)))
#endif
#ifndef CYGHWR_FLASH_28FXXX_PLF_INIT
# define CYGHWR_FLASH_28FXXX_PLF_INIT()
#endif
#ifndef CYGHWR_FLASH_WRITE_ENABLE
#define CYGHWR_FLASH_WRITE_ENABLE()
#endif
#ifndef CYGHWR_FLASH_WRITE_DISABLE
#define CYGHWR_FLASH_WRITE_DISABLE()
#endif

//----------------------------------------------------------------------------
// Now that device properties are defined, include magic for defining
// accessor type and constants.
#include <cyg/io/flash_dev.h>

//----------------------------------------------------------------------------
// Information about supported devices
typedef struct flash_dev_info {
    flash_data_t device_id;
    cyg_uint32   block_size;
    cyg_int32    block_count;
    cyg_uint32   base_mask;
    cyg_uint32   device_size;
    cyg_bool     locking;               // supports locking
    cyg_bool     buffered_w;            // supports buffered writes
    cyg_bool     bootblock;
    cyg_uint32   bootblocks[12];         // 0 is bootblock offset, 1-11 sub-sector sizes (or 0)
    cyg_bool     banked;
    cyg_uint32   banks[2];               // bank offets, highest to lowest (lowest should be 0)
                                         // (only one entry for now, increase to support devices
                                         // with more banks).
} flash_dev_info_t;

static const flash_dev_info_t* flash_dev_info;
static const flash_dev_info_t supported_devices[] = {
#include <cyg/io/flash_28fxxx_parts.inl>
};
#define NUM_DEVICES (sizeof(supported_devices)/sizeof(flash_dev_info_t))

//----------------------------------------------------------------------------
// Functions that put the flash device into non-read mode must reside
// in RAM.
void flash_query(void* data) __attribute__ ((section (".2ram.flash_query")));
int  flash_erase_block(void* block, unsigned int size) 
    __attribute__ ((section (".2ram.flash_erase_block")));
int  flash_program_buf(void* addr, void* data, int len,
                       unsigned long block_mask, int buffer_size)
    __attribute__ ((section (".2ram.flash_program_buf")));
int  flash_lock_block(void* addr)
    __attribute__ ((section (".2ram.flash_lock_block")));
int flash_unlock_block(void* block, int block_size, int blocks)
    __attribute__ ((section (".2ram.flash_unlock_block")));

//----------------------------------------------------------------------------
// Initialize driver details
int
flash_hwr_init(void)
{
    int i;
    flash_data_t id[2];

    CYGHWR_FLASH_28FXXX_PLF_INIT();

    flash_dev_query(id);

    // Look through table for device data
    flash_dev_info = supported_devices;
    for (i = 0; i < NUM_DEVICES; i++) {
        if (flash_dev_info->device_id == id[1])
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
    flash_data_t w;

    ROM = (volatile flash_data_t*) CYGNUM_FLASH_BASE;

    w = ROM[0];

    CYGHWR_FLASH_WRITE_ENABLE();
    
    ROM[0] = FLASH_Read_ID;

    // Manufacturers' code
    id[0] = ROM[0];
    // Part number
    id[1] = ROM[1];

    ROM[0] = FLASH_Reset;

    CYGHWR_FLASH_WRITE_DISABLE();
    
    // Stall, waiting for flash to return to read mode.
    while (w != ROM[0]);
}

//----------------------------------------------------------------------------
// Erase Block
int
flash_erase_block(void* block, unsigned int block_size)
{
    int res = FLASH_ERR_OK;
    int timeout;
    unsigned long len;
    int len_ix = 1;
    flash_data_t stat;
    volatile flash_data_t *ROM;
    volatile flash_data_t *b_p = (flash_data_t*) block;
    volatile flash_data_t *b_v;
    cyg_bool bootblock;

    ROM = FLASH_P2V((unsigned long)block & flash_dev_info->base_mask);

    // Is this the boot sector?
    bootblock = (flash_dev_info->bootblock &&
                 (flash_dev_info->bootblocks[0] == ((unsigned long)block - (unsigned long)ROM)));
    if (bootblock) {
        len = flash_dev_info->bootblocks[len_ix++];
    } else {
        len = flash_dev_info->block_size;
    }

    CYGHWR_FLASH_WRITE_ENABLE();
    
    while (len > 0) {
        b_v = FLASH_P2V(b_p);

        // Clear any error conditions
        ROM[0] = FLASH_Clear_Status;

        // Erase block
        ROM[0] = FLASH_Block_Erase;
        *b_v = FLASH_Confirm;

        timeout = 5000000;
        while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
            if (--timeout == 0) break;
        }
    
        // Restore ROM to "normal" mode
        ROM[0] = FLASH_Reset;

        if (stat & FLASH_ErrorMask) {
            if (!(stat & FLASH_ErrorErase)) {
                res = FLASH_ERR_HWR;    // Unknown error
             } else {
                if (stat & FLASH_ErrorLowVoltage)
                    res = FLASH_ERR_LOW_VOLTAGE;
                else if (stat & FLASH_ErrorLocked)
                    res = FLASH_ERR_PROTECT;
                else
                    res = FLASH_ERR_ERASE;
            }
        }

        // Check if block got erased
        while (len > 0) {
            b_v = FLASH_P2V(b_p++);
            if (*b_v != FLASH_BlankValue ) {
                // Only update return value if erase operation was OK
                if (FLASH_ERR_OK == res) res = FLASH_ERR_DRV_VERIFY;
                return res;
            }
            len -= sizeof(*b_p);
        }

        if (bootblock)
            len = flash_dev_info->bootblocks[len_ix++];
    }

    CYGHWR_FLASH_WRITE_DISABLE();
    
    return res;
}

//----------------------------------------------------------------------------
// Program Buffer
int
flash_program_buf(void* addr, void* data, int len,
                  unsigned long block_mask, int buffer_size)
{
    flash_data_t stat = 0;
    int timeout;

    volatile flash_data_t* ROM;
    volatile flash_data_t* BA;
    volatile flash_data_t* addr_v;
    volatile flash_data_t* addr_p = (flash_data_t*) addr;
    volatile flash_data_t* data_p = (flash_data_t*) data;

    int res = FLASH_ERR_OK;

    // Base address of device(s) being programmed. 
    ROM = FLASH_P2V((unsigned long)addr & flash_dev_info->base_mask);
    BA = FLASH_P2V((unsigned long)addr & ~(flash_dev_info->block_size - 1));

    CYGHWR_FLASH_WRITE_ENABLE();
    
    // Clear any error conditions
    ROM[0] = FLASH_Clear_Status;

#ifdef CYGHWR_DEVS_FLASH_INTEL_BUFFERED_WRITES
    // FIXME: This code has not been adjusted to handle bootblock
    // parts yet.
    // FIXME: This code does not appear to work anymore
    if (0 && flash_dev_info->buffered_w) {
        int i, wc;
        // Write any big chunks first
        while (len >= buffer_size) {
            wc = buffer_size;
            if (wc > len) wc = len;
            len -= wc;
            wc = wc / ((CYGNUM_FLASH_WIDTH/8)*CYGNUM_FLASH_INTERLEAVE);  // Word count
            timeout = 5000000;
            
            *BA = FLASH_Write_Buffer;
            while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
                if (--timeout == 0) {
                    res = FLASH_ERR_DRV_TIMEOUT;
                    goto bad;
                }
                *BA = FLASH_Write_Buffer;
            }
            *BA = FLASHWORD(wc-1);  // Count is 0..N-1
            for (i = 0; i < wc;  i++) {
                addr_v = FLASH_P2V(addr_p++);
                *addr_v = *data_p++;
            }
            *BA = FLASH_Confirm;
            
            ROM[0] = FLASH_Read_Status;
            timeout = 5000000;
            while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
                if (--timeout == 0) {
                    res = FLASH_ERR_DRV_TIMEOUT;
                    goto bad;
                }
            }
        }
    }
#endif // CYGHWR_DEVS_FLASH_INTEL_BUFFERED_WRITES

    while (len > 0) {
        addr_v = FLASH_P2V(addr_p++);
        ROM[0] = FLASH_Program;
        *addr_v = *data_p;
        timeout = 5000000;
        while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
            if (--timeout == 0) {
                res = FLASH_ERR_DRV_TIMEOUT;
                goto bad;
            }
        }
        if (stat & FLASH_ErrorMask) {
            if (!(stat & FLASH_ErrorProgram))
                res = FLASH_ERR_HWR;    // Unknown error
            else {
                if (stat & FLASH_ErrorLowVoltage)
                    res = FLASH_ERR_LOW_VOLTAGE;
                else if (stat & FLASH_ErrorLocked)
                    res = FLASH_ERR_PROTECT;
                else
                    res = FLASH_ERR_PROGRAM;
            }
            break;
        }
        ROM[0] = FLASH_Clear_Status;
        ROM[0] = FLASH_Reset;
        if (*addr_v != *data_p++) {
            res = FLASH_ERR_DRV_VERIFY;
            break;
        }
        len -= sizeof( flash_data_t );
    }

    // Restore ROM to "normal" mode
 bad:
    ROM[0] = FLASH_Reset;            

    CYGHWR_FLASH_WRITE_DISABLE();
    
    // Ideally, we'd want to return not only the failure code, but also
    // the address/device that reported the error.
    return res;
}

#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
//----------------------------------------------------------------------------
// Lock block
int
flash_lock_block(void* block)
{
    volatile flash_data_t *ROM;
    int res = FLASH_ERR_OK;
    flash_data_t state;
    int timeout = 5000000;
    volatile flash_data_t* b_p = (flash_data_t*) block;
    volatile flash_data_t *b_v;
    cyg_bool bootblock;
    int len, len_ix = 1;

    if (!flash_dev_info->locking)
        return res;

#ifdef DEBUG
    d_print("flash_lock_block %08x\n", block);
#endif

    ROM = (volatile flash_data_t*)((unsigned long)block & flash_dev_info->base_mask);

    // Is this the boot sector?
    bootblock = (flash_dev_info->bootblock &&
                 (flash_dev_info->bootblocks[0] == ((unsigned long)block - (unsigned long)ROM)));
    if (bootblock) {
        len = flash_dev_info->bootblocks[len_ix++];
    } else {
        len = flash_dev_info->block_size;
    }

    CYGHWR_FLASH_WRITE_ENABLE();
    
    while (len > 0) {
        b_v = FLASH_P2V(b_p);

        // Clear any error conditions
        ROM[0] = FLASH_Clear_Status;

        // Set lock bit
        *b_v = FLASH_Set_Lock;
        *b_v = FLASH_Set_Lock_Confirm;  // Confirmation
        while(((state = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
            if (--timeout == 0) {
                res = FLASH_ERR_DRV_TIMEOUT;
                break;
            }
        }

        // Restore ROM to "normal" mode
        ROM[0] = FLASH_Reset;

        // Go to next block
        b_p += len / sizeof( flash_data_t );
        len = 0;

        if (FLASH_ErrorLock == (state & FLASH_ErrorLock))
            res = FLASH_ERR_LOCK;

        if (res != FLASH_ERR_OK)
            break;
            
        if (bootblock)
            len = flash_dev_info->bootblocks[len_ix++];
    }

    CYGHWR_FLASH_WRITE_DISABLE();
    
    return res;
}

//----------------------------------------------------------------------------
// Unlock block

int
flash_unlock_block(void* block, int block_size, int blocks)
{
    volatile flash_data_t *ROM;
    int res = FLASH_ERR_OK;
    flash_data_t state;
    int timeout = 5000000;
    volatile flash_data_t* b_p = (flash_data_t*) block;
    volatile flash_data_t *b_v;

#if (defined(CYGHWR_DEVS_FLASH_SHARP_LH28F016SCT_Z4) || defined(CYGHWR_DEVS_FLASH_SHARP_LH28F016SCT_95) )
    // The Sharp device follows all the same rules as the Intel 28x part,
    // except that the unlocking mechanism unlocks all blocks at once.  This
    // is the way the Strata part seems to work.  I will replace the 
    // flash_unlock_block function with one similar to the Strata function.
    // As the Sharp part does not have the bootlock characteristics, I
    // will ignore them.
//
// The difficulty with this operation is that the hardware does not support
// unlocking single blocks.  However, the logical layer would like this to
// be the case, so this routine emulates it.  The hardware can clear all of
// the locks in the device at once.  This routine will use that approach and
// then reset the regions which are known to be locked.
//

#define MAX_FLASH_BLOCKS (flash_dev_info->block_count * CYGNUM_FLASH_SERIES)

    unsigned char is_locked[MAX_FLASH_BLOCKS];
    int i;

    // Get base address and map addresses to virtual addresses
#ifdef DEBUG
    d_print("\nNow inside low level driver\n");
#endif
    ROM = (volatile flash_data_t*) CYGNUM_FLASH_BASE;
    block = FLASH_P2V(block);

    // Clear any error conditions
    ROM[0] = FLASH_Clear_Status;

    // Get current block lock state.  This needs to access each block on
    // the device so currently locked blocks can be re-locked.
    b_p = ROM;
    for (i = 0;  i < blocks;  i++) {
        b_v = FLASH_P2V( b_p );
        *b_v = FLASH_Read_ID;
        if (b_v == block) {
            is_locked[i] = 0;
        } else {
            if(b_v[2]){ /* it is possible that one of the interleaved devices
                         * is locked, but others are not.  Coming out of this
                         * function, if one was locked, all will be locked.
                         */
                is_locked[i] = 1;
            }else{
                is_locked[i] = 0;
            }
        }
#ifdef DEBUG
#endif
        b_p += block_size / sizeof(*b_p);
    }
    ROM[0] = FLASH_Reset;
#ifdef DEBUG
    for (i = 0;  i < blocks;  i++) {
        d_print("\nblock %d  %s", i,
                is_locked[i] ? "LOCKED" : "UNLOCKED");
    }
    d_print("\n");
#endif

    // Clears all lock bits
    ROM[0] = FLASH_Clear_Lock;
    ROM[0] = FLASH_Clear_Lock_Confirm;  // Confirmation
    timeout = 5000000;
    while(((state = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
        if (--timeout == 0) break;
    }

    // Restore the lock state
    b_p = ROM;
    for (i = 0;  i < blocks;  i++) {
        b_v = FLASH_P2V( b_p );
        if (is_locked[i]) {
            *b_v = FLASH_Set_Lock;
            *b_v = FLASH_Set_Lock_Confirm;  // Confirmation
            timeout = 5000000;
            while(((state = ROM[0]) & FLASH_Status_Ready)
                  != FLASH_Status_Ready) {
                if (--timeout == 0){
                    res = FLASH_ERR_DRV_TIMEOUT;
                    break;
                }
            }
            if (FLASH_ErrorLock == (state & FLASH_ErrorLock))
                res = FLASH_ERR_LOCK;
            
            if (res != FLASH_ERR_OK)
                break;

        }
        b_p += block_size / sizeof(*b_p);
    }

    // Restore ROM to "normal" mode
    ROM[0] = FLASH_Reset;

    return res;

#else // not CYGHWR_DEVS_FLASH_SHARP_LH28F016SCT_Z4

    cyg_bool bootblock;
    int len, len_ix = 1;

    if (!flash_dev_info->locking)
        return res;

    ROM = (volatile flash_data_t*)((unsigned long)block & flash_dev_info->base_mask);

#ifdef DEBUG
    d_print("flash_unlock_block dev %08x block %08x size %08x count %08x\n", ROM, block, block_size, blocks);
#endif

    // Is this the boot sector?
    bootblock = (flash_dev_info->bootblock &&
                 (flash_dev_info->bootblocks[0] == ((unsigned long)block - (unsigned long)ROM)));
    if (bootblock) {
        len = flash_dev_info->bootblocks[len_ix++];
    } else {
        len = flash_dev_info->block_size;
    }

    CYGHWR_FLASH_WRITE_ENABLE();
    
    while (len > 0) {

        b_v = FLASH_P2V(b_p);

        // Clear any error conditions
        ROM[0] = FLASH_Clear_Status;

        // Clear lock bit
        *b_v = FLASH_Clear_Lock;
        *b_v = FLASH_Clear_Lock_Confirm;  // Confirmation
        while(((state = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
            if (--timeout == 0) {
                res = FLASH_ERR_DRV_TIMEOUT;
                break;
            }
        }

        // Restore ROM to "normal" mode
        ROM[0] = FLASH_Reset;

        // Go to next block
        b_p += len / sizeof( flash_data_t );
        len = 0;

        if (FLASH_ErrorLock == (state & FLASH_ErrorLock))
            res = FLASH_ERR_LOCK;

        if (res != FLASH_ERR_OK)
            break;

        if (bootblock)
            len = flash_dev_info->bootblocks[len_ix++];
    }

    CYGHWR_FLASH_WRITE_DISABLE();
    
    return res;

    // FIXME: Unlocking need to support some other parts in the future
    // as well which take a little more diddling.
#if 0
//
// The difficulty with this operation is that the hardware does not support
// unlocking single blocks.  However, the logical layer would like this to
// be the case, so this routine emulates it.  The hardware can clear all of
// the locks in the device at once.  This routine will use that approach and
// then reset the regions which are known to be locked.
//

#define MAX_FLASH_BLOCKS (flash_dev_info->block_count * CYGNUM_FLASH_SERIES)

    unsigned char is_locked[MAX_FLASH_BLOCKS];

    // Get base address and map addresses to virtual addresses
    ROM = FLASH_P2V( CYGNUM_FLASH_BASE_MASK & (unsigned int)block );
    block = FLASH_P2V(block);

    // Clear any error conditions
    ROM[0] = FLASH_Clear_Status;

    // Get current block lock state.  This needs to access each block on
    // the device so currently locked blocks can be re-locked.
    bp = ROM;
    for (i = 0;  i < blocks;  i++) {
        bpv = FLASH_P2V( bp );
        *bpv = FLASH_Read_Query;
        if (bpv == block) {
            is_locked[i] = 0;
        } else {
            is_locked[i] = bpv[2];
        }
        bp += block_size / sizeof(*bp);
    }

    // Clears all lock bits
    ROM[0] = FLASH_Clear_Locks;
    ROM[0] = FLASH_Clear_Locks_Confirm;  // Confirmation
    timeout = 5000000;
    while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
        if (--timeout == 0) break;
    }

    // Restore the lock state
    bp = ROM;
    for (i = 0;  i < blocks;  i++) {
        bpv = FLASH_P2V( bp );
        if (is_locked[i]) {
            *bpv = FLASH_Set_Lock;
            *bpv = FLASH_Set_Lock_Confirm;  // Confirmation
            timeout = 5000000;
            while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
                if (--timeout == 0) break;
            }
        }
        bp += block_size / sizeof(*bp);
    }

    // Restore ROM to "normal" mode
    ROM[0] = FLASH_Reset;
#endif
#endif // #CYGHWR_DEVS_FLASH_SHARP_LH28F016SCT_Z4
}
#endif // CYGHWR_IO_FLASH_BLOCK_LOCKING

#endif // CYGONCE_DEVS_FLASH_INTEL_28FXXX_INL
