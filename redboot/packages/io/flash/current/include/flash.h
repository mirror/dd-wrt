//==========================================================================
//
//      flash.h
//
//      Flash programming - external interfaces
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
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef _IO_FLASH_H_
#define _IO_FLASH_H_

#include <pkgconf/io_flash.h>
#include <cyg/hal/hal_cache.h>

typedef int _printf(const char *fmt, ...);

externC int flash_init(_printf *pf);
externC int flash_erase(void *base, int len, void **err_address);
externC int flash_program(void *flash_base, void *ram_base, int len, void **err_address);
externC int flash_read(void *flash_base, void *ram_base, int len, void **err_address);
externC void flash_dev_query(void *data);
#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
externC int flash_lock(void *base, int len, void **err_address);
externC int flash_unlock(void *base, int len, void **err_address);
#endif
externC int flash_verify_addr(void *base);
externC int flash_get_limits(void *base, void **start, void **end);
externC int flash_get_block_info(int *block_size, int *blocks);
externC bool flash_code_overlaps(void *start, void *end);
externC char *flash_errmsg(int err);

#define FLASH_ERR_OK              0x00  // No error - operation complete
#define FLASH_ERR_INVALID         0x01  // Invalid FLASH address
#define FLASH_ERR_ERASE           0x02  // Error trying to erase
#define FLASH_ERR_LOCK            0x03  // Error trying to lock/unlock
#define FLASH_ERR_PROGRAM         0x04  // Error trying to program
#define FLASH_ERR_PROTOCOL        0x05  // Generic error
#define FLASH_ERR_PROTECT         0x06  // Device/region is write-protected
#define FLASH_ERR_NOT_INIT        0x07  // FLASH info not yet initialized
#define FLASH_ERR_HWR             0x08  // Hardware (configuration?) problem
#define FLASH_ERR_ERASE_SUSPEND   0x09  // Device is in erase suspend mode
#define FLASH_ERR_PROGRAM_SUSPEND 0x0a  // Device is in in program suspend mode
#define FLASH_ERR_DRV_VERIFY      0x0b  // Driver failed to verify data
#define FLASH_ERR_DRV_TIMEOUT     0x0c  // Driver timed out waiting for device
#define FLASH_ERR_DRV_WRONG_PART  0x0d  // Driver does not support device
#define FLASH_ERR_LOW_VOLTAGE     0x0e  // Not enough juice to complete job

#ifdef CYGPKG_IO_FLASH_BLOCK_DEVICE
typedef struct {
    CYG_ADDRESS offset;
    int len;
    int flasherr;
    void **err_address;
} cyg_io_flash_getconfig_erase_t;

typedef struct {
    int dev_size;
} cyg_io_flash_getconfig_devsize_t;

typedef struct {
    CYG_ADDRESS offset;
    int block_size;
} cyg_io_flash_getconfig_blocksize_t;
#endif

#ifdef _FLASH_PRIVATE_

struct flash_info {
    int   block_size;   // Assuming fixed size "blocks"
    int   blocks;       // Number of blocks
    int   buffer_size;  // Size of write buffer (only defined for some devices)
    unsigned long block_mask;
    void *start, *end;  // Address range
    int   init;
    _printf *pf;
};

externC struct flash_info flash_info;
externC int  flash_hwr_init(void);
externC int  flash_hwr_map_error(int err);

// 
// Some FLASH devices may require additional support, e.g. to turn on
// appropriate voltage drivers, before any operation.
//
#ifdef  CYGIMP_FLASH_ENABLE
#define FLASH_Enable CYGIMP_FLASH_ENABLE
extern void CYGIMP_FLASH_ENABLE(void *, void *);
#else
#define FLASH_Enable(_start_, _end_)
#endif
#ifdef  CYGIMP_FLASH_DISABLE
#define FLASH_Disable CYGIMP_FLASH_DISABLE
extern void CYGIMP_FLASH_DISABLE(void *, void *);
#else
#define FLASH_Disable(_start_, _end_)
#endif

//
// Some platforms have a DIP switch or jumper that tells the software that
// the flash is write protected.
//
#ifdef CYGSEM_IO_FLASH_SOFT_WRITE_PROTECT
externC cyg_bool plf_flash_query_soft_wp(void *addr, int len);
#endif

//---------------------------------------------------------------------------
// Execution of flash code must be done inside a
// HAL_FLASH_CACHES_OFF/HAL_FLASH_CACHES_ON region - disabling the
// cache on unified cache systems is necessary to prevent burst access
// to the flash area being programmed. With Harvard style caches, only
// the data cache needs to be disabled, but the instruction cache is
// disabled for consistency.

// Targets may provide alternative implementations for these macros in
// the hal_cache.h (or var/plf) files.

// The first part below is a generic, optimal implementation.  The
// second part is the old implementation that has been tested to work
// on some targets - but it is not be suitable for targets that would
// do burst access to the flash (it does not disable the data cache).

// Both implementations must be called with interrupts disabled.

// NOTE: Do _not_ change any of the below macros without checking that
//       the changed code still works on _all_ platforms that rely on these
//       macros. There is no such thing as logical and correct when dealing
//       with different cache and IO models, so _do not_ mess with this code
//       unless you test it properly afterwards.

#ifndef HAL_FLASH_CACHES_OFF

// Some drivers have only been tested with the old macros below.
#ifndef HAL_FLASH_CACHES_OLD_MACROS

#ifdef HAL_CACHE_UNIFIED

// Note: the ucache code has not been tested yet on any target.
#define HAL_FLASH_CACHES_OFF(_d_, _i_)          \
    CYG_MACRO_START                             \
    _i_ = 0; /* avoids warning */               \
    HAL_UCACHE_IS_ENABLED(_d_);                 \
    HAL_UCACHE_SYNC();                          \
    HAL_UCACHE_INVALIDATE_ALL();                \
    HAL_UCACHE_DISABLE();                       \
    CYG_MACRO_END

#define HAL_FLASH_CACHES_ON(_d_, _i_)           \
    CYG_MACRO_START                             \
    if (_d_) HAL_UCACHE_ENABLE();               \
    CYG_MACRO_END

#else  // HAL_CACHE_UNIFIED

#define HAL_FLASH_CACHES_OFF(_d_, _i_)          \
    CYG_MACRO_START                             \
    _i_ = 0; /* avoids warning */               \
    HAL_DCACHE_IS_ENABLED(_d_);                 \
    HAL_DCACHE_SYNC();                          \
    HAL_DCACHE_INVALIDATE_ALL();                \
    HAL_DCACHE_DISABLE();                       \
    HAL_ICACHE_INVALIDATE_ALL();                \
    CYG_MACRO_END

#define HAL_FLASH_CACHES_ON(_d_, _i_)           \
    CYG_MACRO_START                             \
    if (_d_) HAL_DCACHE_ENABLE();               \
    CYG_MACRO_END

#endif // HAL_CACHE_UNIFIED

#else  // HAL_FLASH_CACHES_OLD_MACROS

// Note: This implementation is broken as it will always enable the i-cache
//       even if it was not enabled before. It also doesn't work if the
//       target uses burst access to flash since the d-cache is left enabled.
//       However, this does not mean you can change this code! Leave it as
//       is - if you want a different implementation, provide it in the
//       arch/var/platform cache header file.

#define HAL_FLASH_CACHES_OFF(_d_, _i_)          \
    _d_ = 0; /* avoids warning */               \
    _i_ = 0; /* avoids warning */               \
    HAL_DCACHE_SYNC();                          \
    HAL_ICACHE_DISABLE();

#define HAL_FLASH_CACHES_ON(_d_, _i_)           \
    HAL_ICACHE_ENABLE();

#endif  // HAL_FLASH_CACHES_OLD_MACROS

#endif  // HAL_FLASH_CACHES_OFF

#endif  // _FLASH_PRIVATE_

#endif  // _IO_FLASH_H_
