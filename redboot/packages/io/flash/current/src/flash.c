//==========================================================================
//
//      flash.c
//
//      Flash programming
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-07-26
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_flash.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <string.h>

#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>

// When this flag is set, do not actually jump to the relocated code.
// This can be used for running the function in place (RAM startup only),
// allowing calls to diag_printf() and similar.
#undef RAM_FLASH_DEV_DEBUG
#if !defined(CYG_HAL_STARTUP_RAM) && defined(RAM_FLASH_DEV_DEBUG)
# warning "Can only enable the flash debugging when configured for RAM startup"
#endif

struct flash_info flash_info;

// These are the functions in the HW specific driver we need to call.
typedef void code_fun(void*);

externC code_fun flash_query;
externC code_fun flash_erase_block;
externC code_fun flash_program_buf;
externC code_fun flash_read_buf;
externC code_fun flash_lock_block;
externC code_fun flash_unlock_block;

int
flash_init(_printf *pf)
{
    int err;

    if (flash_info.init) return FLASH_ERR_OK;
    flash_info.pf = pf; // Do this before calling into the driver
    if ((err = flash_hwr_init()) != FLASH_ERR_OK) {
        return err;
    }
    flash_info.block_mask = ~(flash_info.block_size-1);
    flash_info.init = 1;
    return FLASH_ERR_OK;
}

// Use this function to make function pointers anonymous - forcing the
// compiler to use jumps instead of branches when calling driver
// services.
static void* __anonymizer(void* p)
{
  return p;
}

// FIXME: Want to change all drivers to use this function. But it may
// make sense to wait till device structure pointer arguments get
// added as well.
void
flash_dev_query(void* data)
{
    typedef void code_fun(void*);
    code_fun *_flash_query;
    int d_cache, i_cache;

    _flash_query = (code_fun*) __anonymizer(&flash_query);

    HAL_FLASH_CACHES_OFF(d_cache, i_cache);
    (*_flash_query)(data);
    HAL_FLASH_CACHES_ON(d_cache, i_cache);
}

int
flash_verify_addr(void *target)
{
    if (!flash_info.init) {
        return FLASH_ERR_NOT_INIT;
    }
    if (((CYG_ADDRESS)target >= (CYG_ADDRESS)flash_info.start) &&
        ((CYG_ADDRESS)target <= ( ((CYG_ADDRESS)flash_info.end) - 1) )) {
        return FLASH_ERR_OK;
    } else {
        return FLASH_ERR_INVALID;
    }
}

int
flash_get_limits(void *target, void **start, void **end)
{
    if (!flash_info.init) {
        return FLASH_ERR_NOT_INIT;
    }
    *start = flash_info.start;
    *end = flash_info.end;
    return FLASH_ERR_OK;
}

int
flash_get_block_info(int *block_size, int *blocks)
{
    if (!flash_info.init) {
        return FLASH_ERR_NOT_INIT;
    }
    *block_size = flash_info.block_size;
    *blocks = flash_info.blocks;
    return FLASH_ERR_OK;
}

int
flash_erase(void *addr, int len, void **err_addr)
{
    unsigned short *block, *end_addr;
    int stat = 0;
    typedef int code_fun(unsigned short *, unsigned int);
    code_fun *_flash_erase_block;
    int d_cache, i_cache;

    if (!flash_info.init) {
        return FLASH_ERR_NOT_INIT;
    }

#ifdef CYGSEM_IO_FLASH_SOFT_WRITE_PROTECT
    if (plf_flash_query_soft_wp(addr,len))
        return FLASH_ERR_PROTECT;
#endif

     _flash_erase_block = (code_fun*) __anonymizer(&flash_erase_block);

    block = (unsigned short *)((CYG_ADDRESS)addr & flash_info.block_mask);
    end_addr = (unsigned short *)((CYG_ADDRESS)addr+len);

    /* Check to see if end_addr overflowed */
    if( (end_addr < block) && (len > 0) ){
        end_addr = (unsigned short *) ((CYG_ADDRESS) flash_info.end - 1);
    }

#ifdef CYGSEM_IO_FLASH_CHATTER
    (*flash_info.pf)("... Erase from %p-%p: ", (void*)block, (void*)end_addr);
#endif

    HAL_FLASH_CACHES_OFF(d_cache, i_cache);
    FLASH_Enable(block, end_addr);
    while (block < end_addr) {
        // Supply the blocksize for a gross check for erase success
        unsigned short *tmp_block;
#if !defined(CYGSEM_IO_FLASH_READ_INDIRECT)
        int i;
        unsigned char *dp;
        bool erased = true;

        dp = (unsigned char *)block;
        for (i = 0;  i < flash_info.block_size;  i++) {
            if (*dp++ != (unsigned char)0xFF) {
                erased = false;
                break;
            }
        }
#else
        bool erased = false;
#endif

        if (!erased) {
            stat = (*_flash_erase_block)(block, flash_info.block_size);
            stat = flash_hwr_map_error(stat);
        }
        if (stat) {
            *err_addr = (void *)block;
            break;
        }

        // Check to see if block will overflow
        tmp_block = block + flash_info.block_size / sizeof(*block);
        if(tmp_block < block){
            // If block address overflows, set block value to end on this loop
            block = end_addr;
        }
        else{
            block = tmp_block;
        }
#ifdef CYGSEM_IO_FLASH_CHATTER
        (*flash_info.pf)(".");
#endif
    }
    FLASH_Disable(block, end_addr);
    HAL_FLASH_CACHES_ON(d_cache, i_cache);
#ifdef CYGSEM_IO_FLASH_CHATTER
    (*flash_info.pf)("\n");
#endif
    return (stat);
}

int
flash_program(void *_addr, void *_data, int len, void **err_addr)
{
    int stat = 0;
    int size;
    typedef int code_fun(void *, void *, int, unsigned long, int);
    code_fun *_flash_program_buf;
    unsigned char *addr = (unsigned char *)_addr;
    unsigned char *data = (unsigned char *)_data;
    CYG_ADDRESS tmp;
    int d_cache, i_cache;

    if (!flash_info.init) {
        return FLASH_ERR_NOT_INIT;
    }

#ifdef CYGSEM_IO_FLASH_SOFT_WRITE_PROTECT
    if (plf_flash_query_soft_wp(addr,len))
        return FLASH_ERR_PROTECT;
#endif

    _flash_program_buf = (code_fun*) __anonymizer(&flash_program_buf);

#ifdef CYGSEM_IO_FLASH_CHATTER
    (*flash_info.pf)("... Program from %p-%p at %p: ", (void*)data, 
                     (void*)(((CYG_ADDRESS)data)+len), (void*)addr);
#endif

    HAL_FLASH_CACHES_OFF(d_cache, i_cache);
    FLASH_Enable((unsigned short*)addr, (unsigned short *)(addr+len));
    while (len > 0) {
        size = len;
        if (size > flash_info.block_size) size = flash_info.block_size;

        tmp = (CYG_ADDRESS)addr & ~flash_info.block_mask;
        if (tmp) {
                tmp = flash_info.block_size - tmp;
                if (size>tmp) size = tmp;

        }

        stat = (*_flash_program_buf)(addr, data, size, 
                                     flash_info.block_mask, flash_info.buffer_size);
        stat = flash_hwr_map_error(stat);
#ifdef CYGSEM_IO_FLASH_VERIFY_PROGRAM
        if (0 == stat) // Claims to be OK
            if (memcmp(addr, data, size) != 0) {                
                stat = 0x0BAD;
#ifdef CYGSEM_IO_FLASH_CHATTER
                (*flash_info.pf)("V");
#endif
            }
#endif
        if (stat) {
            *err_addr = (void *)addr;
            break;
        }
#ifdef CYGSEM_IO_FLASH_CHATTER
        (*flash_info.pf)(".");
#endif
        len -= size;
        addr += size/sizeof(*addr);
        data += size/sizeof(*data);
    }
    FLASH_Disable((unsigned short*)addr, (unsigned short *)(addr+len));
    HAL_FLASH_CACHES_ON(d_cache, i_cache);
#ifdef CYGSEM_IO_FLASH_CHATTER
    (*flash_info.pf)("\n");
#endif
    return (stat);
}

int
flash_read(void *_addr, void *_data, int len, void **err_addr)
{
#ifdef CYGSEM_IO_FLASH_READ_INDIRECT
    int stat = 0;
    int size;
    typedef int code_fun(void *, void *, int, unsigned long, int);
    code_fun *_flash_read_buf;
    unsigned char *addr = (unsigned char *)_addr;
    unsigned char *data = (unsigned char *)_data;
    CYG_ADDRESS tmp;
    int d_cache, i_cache;

    if (!flash_info.init) {
        return FLASH_ERR_NOT_INIT;
    }

    _flash_read_buf = (code_fun*) __anonymizer(&flash_read_buf);

#ifdef CYGSEM_IO_FLASH_CHATTER
    (*flash_info.pf)("... Read from %p-%p at %p: ", (void*)data, 
                     (void*)(((CYG_ADDRESS)data)+len), (void*)addr);
#endif

    HAL_FLASH_CACHES_OFF(d_cache, i_cache);
    FLASH_Enable((unsigned short*)addr, (unsigned short *)(addr+len));
    while (len > 0) {
        size = len;
        if (size > flash_info.block_size) size = flash_info.block_size;

        tmp = (CYG_ADDRESS)addr & ~flash_info.block_mask;
        if (tmp) {
                tmp = flash_info.block_size - tmp;
                if (size>tmp) size = tmp;

        }

        stat = (*_flash_read_buf)(addr, data, size, 
                                     flash_info.block_mask, flash_info.buffer_size);
        stat = flash_hwr_map_error(stat);
#ifdef CYGSEM_IO_FLASH_VERIFY_PROGRAM_
        if (0 == stat) // Claims to be OK
            if (memcmp(addr, data, size) != 0) {                
                stat = 0x0BAD;
#ifdef CYGSEM_IO_FLASH_CHATTER
                (*flash_info.pf)("V");
#endif
            }
#endif
        if (stat) {
            *err_addr = (void *)addr;
            break;
        }
#ifdef CYGSEM_IO_FLASH_CHATTER
        (*flash_info.pf)(".");
#endif
        len -= size;
        addr += size/sizeof(*addr);
        data += size/sizeof(*data);
    }
    FLASH_Disable((unsigned short*)addr, (unsigned short *)(addr+len));
    HAL_FLASH_CACHES_ON(d_cache, i_cache);
#ifdef CYGSEM_IO_FLASH_CHATTER
    (*flash_info.pf)("\n");
#endif
    return (stat);
#else // CYGSEM_IO_FLASH_READ_INDIRECT
    // Direct access to FLASH memory is possible - just move the requested bytes
    if (!flash_info.init) {
        return FLASH_ERR_NOT_INIT;
    }
    memcpy(_data, _addr, len);
    return FLASH_ERR_OK;
#endif
}

#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING

int
flash_lock(void *addr, int len, void **err_addr)
{
    unsigned short *block, *end_addr;
    int stat = 0;
    typedef int code_fun(unsigned short *);
    code_fun *_flash_lock_block;
    int d_cache, i_cache;

    if (!flash_info.init) {
        return FLASH_ERR_NOT_INIT;
    }

#ifdef CYGSEM_IO_FLASH_SOFT_WRITE_PROTECT
    if (plf_flash_query_soft_wp(addr,len))
        return FLASH_ERR_PROTECT;
#endif

    _flash_lock_block = (code_fun*) __anonymizer(&flash_lock_block);

    block = (unsigned short *)((CYG_ADDRESS)addr & flash_info.block_mask);
    end_addr = (unsigned short *)((CYG_ADDRESS)addr+len);

    /* Check to see if end_addr overflowed */
    if( (end_addr < block) && (len > 0) ){
        end_addr = (unsigned short *) ((CYG_ADDRESS) flash_info.end - 1);
    }

#ifdef CYGSEM_IO_FLASH_CHATTER
    (*flash_info.pf)("... Lock from %p-%p: ", block, end_addr);
#endif

    HAL_FLASH_CACHES_OFF(d_cache, i_cache);
    FLASH_Enable(block, end_addr);
    while (block < end_addr) {
        unsigned short *tmp_block;
        stat = (*_flash_lock_block)(block);
        stat = flash_hwr_map_error(stat);
        if (stat) {
            *err_addr = (void *)block;
            break;
        }

        // Check to see if block will overflow
        tmp_block = block + flash_info.block_size / sizeof(*block);
        if(tmp_block < block){
            // If block address overflows, set block value to end on this loop
            block = end_addr;
        }
        else{
            block = tmp_block;
        }
#ifdef CYGSEM_IO_FLASH_CHATTER
        (*flash_info.pf)(".");
#endif
    }
    FLASH_Disable(block, end_addr);
    HAL_FLASH_CACHES_ON(d_cache, i_cache);
#ifdef CYGSEM_IO_FLASH_CHATTER
    (*flash_info.pf)("\n");
#endif
    return (stat);
}

int
flash_unlock(void *addr, int len, void **err_addr)
{
    unsigned short *block, *end_addr;
    int stat = 0;
    typedef int code_fun(unsigned short *, int, int);
    code_fun *_flash_unlock_block;
    int d_cache, i_cache;

    if (!flash_info.init) {
        return FLASH_ERR_NOT_INIT;
    }

#ifdef CYGSEM_IO_FLASH_SOFT_WRITE_PROTECT
    if (plf_flash_query_soft_wp(addr,len))
        return FLASH_ERR_PROTECT;
#endif

    _flash_unlock_block = (code_fun*) __anonymizer(&flash_unlock_block);

    block = (unsigned short *)((CYG_ADDRESS)addr & flash_info.block_mask);
    end_addr = (unsigned short *)((CYG_ADDRESS)addr+len);

    /* Check to see if end_addr overflowed */
    if( (end_addr < block) && (len > 0) ){
        end_addr = (unsigned short *) ((CYG_ADDRESS) flash_info.end - 1);
    }

#ifdef CYGSEM_IO_FLASH_CHATTER
    (*flash_info.pf)("... Unlock from %p-%p: ", block, end_addr);
#endif

    HAL_FLASH_CACHES_OFF(d_cache, i_cache);
    FLASH_Enable(block, end_addr);
    while (block < end_addr) {
        unsigned short *tmp_block;
        stat = (*_flash_unlock_block)(block, flash_info.block_size, flash_info.blocks);
        stat = flash_hwr_map_error(stat);
        if (stat) {
            *err_addr = (void *)block;
            break;
        }

        tmp_block = block + flash_info.block_size / sizeof(*block);
        if(tmp_block < block){
            // If block address overflows, set block value to end on this loop
            block = end_addr;
        }
        else{
            block = tmp_block;
        }
#ifdef CYGSEM_IO_FLASH_CHATTER
        (*flash_info.pf)(".");
#endif
    }
    FLASH_Disable(block, end_addr);
    HAL_FLASH_CACHES_ON(d_cache, i_cache);
#ifdef CYGSEM_IO_FLASH_CHATTER
    (*flash_info.pf)("\n");
#endif
    return (stat);
}
#endif

char *
flash_errmsg(int err)
{
    switch (err) {
    case FLASH_ERR_OK:
        return "No error - operation complete";
    case FLASH_ERR_ERASE_SUSPEND:
        return "Device is in erase suspend state";
    case FLASH_ERR_PROGRAM_SUSPEND:
        return "Device is in program suspend state";
    case FLASH_ERR_INVALID:
        return "Invalid FLASH address";
    case FLASH_ERR_ERASE:
        return "Error trying to erase";
    case FLASH_ERR_LOCK:
        return "Error trying to lock/unlock";
    case FLASH_ERR_PROGRAM:
        return "Error trying to program";
    case FLASH_ERR_PROTOCOL:
        return "Generic error";
    case FLASH_ERR_PROTECT:
        return "Device/region is write-protected";
    case FLASH_ERR_NOT_INIT:
        return "FLASH sub-system not initialized";
    case FLASH_ERR_DRV_VERIFY:
        return "Data verify failed after operation";
    case FLASH_ERR_DRV_TIMEOUT:
        return "Driver timed out waiting for device";
    case FLASH_ERR_DRV_WRONG_PART:
        return "Driver does not support device";
    case FLASH_ERR_LOW_VOLTAGE:
        return "Device reports low voltage";
    default:
        return "Unknown error";
    }
}

// EOF io/flash/..../flash.c
