//==========================================================================
//
//      synth.c
//
//      Flash programming
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
// Author(s):    andrew.lunn@ascom.ch
// Contributors: jlarmour
// Date:         2001-10-30
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/devs_flash_synth.h>

#include <cyg/hal/hal_io.h>
#include <cyg/infra/cyg_ass.h>
#include <errno.h>
#include <string.h>

#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>

#include "synth.h"

/* Holds the fd for the flash file */
int cyg_dev_flash_synth_flashfd;

/* Holds the base address of the mmap'd region */
flash_t *cyg_dev_flash_synth_base;

/* Helper function. The Linux system call cannot pass 6 parameters. Instead
   a structure is filled in and passed as one parameter */
static int 
cyg_hal_sys_do_mmap(void *addr, unsigned long length, unsigned long prot, 
                    unsigned long flags, unsigned long fd, unsigned long off)
{

    struct cyg_hal_sys_mmap_args args;
  
    args.addr = (unsigned long) addr;
    args.len = length;
    args.prot = prot = prot;
    args.flags = flags;
    args.fd = fd;
    args.offset = off;

    return (cyg_hal_sys_mmap(&args));
}           

int
flash_hwr_init(void)
{
    flash_info.block_size = CYGNUM_FLASH_SYNTH_BLOCKSIZE;
    flash_info.buffer_size = 0;
    flash_info.blocks = CYGNUM_FLASH_SYNTH_NUMBLOCKS;

    cyg_dev_flash_synth_flashfd = cyg_hal_sys_open(CYGDAT_FLASH_SYNTH_FILENAME, 
                CYG_HAL_SYS_O_RDWR, 
                CYG_HAL_SYS_S_IRWXU|CYG_HAL_SYS_S_IRWXG|CYG_HAL_SYS_S_IRWXO);
    if (cyg_dev_flash_synth_flashfd == -ENOENT) {
        long w, bytesleft;
        char buf[128];

        cyg_dev_flash_synth_flashfd = cyg_hal_sys_open(
                CYGDAT_FLASH_SYNTH_FILENAME, 
                CYG_HAL_SYS_O_RDWR|CYG_HAL_SYS_O_CREAT, 
                CYG_HAL_SYS_S_IRWXU|CYG_HAL_SYS_S_IRWXG|CYG_HAL_SYS_S_IRWXO);
        CYG_ASSERT( cyg_dev_flash_synth_flashfd >= 0, 
                    "Opening of the file for the synth flash failed!");
        // fill with 0xff
        memset( buf, 0xff, sizeof(buf) );
        bytesleft = CYGNUM_FLASH_SYNTH_BLOCKSIZE * CYGNUM_FLASH_SYNTH_NUMBLOCKS;
        while (bytesleft > 0)
        {
            int bytesneeded;
            bytesneeded = bytesleft < sizeof(buf) ?  bytesleft : sizeof(buf);

            w = cyg_hal_sys_write( cyg_dev_flash_synth_flashfd, buf,
                                   bytesneeded );
            CYG_ASSERT(w == bytesneeded, "initialization of flash file failed");
            bytesleft -= bytesneeded;
        } // while
    }
    CYG_ASSERT( cyg_dev_flash_synth_flashfd >= 0, 
                "Opening of the file for the synth flash failed!");
    if ( cyg_dev_flash_synth_flashfd <= 0 ) {
        return FLASH_ERR_HWR;
    }
    cyg_dev_flash_synth_base = (flash_t *)cyg_hal_sys_do_mmap( 
#ifdef CYGMEM_FLASH_SYNTH_BASE
                CYGMEM_FLASH_SYNTH_BASE,
#else
                NULL,
#endif
                (CYGNUM_FLASH_SYNTH_BLOCKSIZE * CYGNUM_FLASH_SYNTH_NUMBLOCKS),
                CYG_HAL_SYS_PROT_READ, 
#ifdef CYGSEM_FLASH_SYNTH_FILE_WRITEBACK
                CYG_HAL_SYS_MAP_SHARED
#else
                CYG_HAL_SYS_MAP_PRIVATE
#endif
#ifdef CYGMEM_FLASH_SYNTH_BASE
                |CYG_HAL_SYS_MAP_FIXED
#endif
                , cyg_dev_flash_synth_flashfd, 0 );
    CYG_ASSERT( cyg_dev_flash_synth_base > 0, "mmap of flash file failed!" );

    if (cyg_dev_flash_synth_base <= 0) {
        return FLASH_ERR_HWR;
    }
    flash_info.start = cyg_dev_flash_synth_base;
    flash_info.end = (void *)(((char *)cyg_dev_flash_synth_base) +
        (CYGNUM_FLASH_SYNTH_BLOCKSIZE * CYGNUM_FLASH_SYNTH_NUMBLOCKS));

    return FLASH_ERR_OK;
}

// Map a hardware status to a package error
int
flash_hwr_map_error(int err)
{
    return err;
}

// See if a range of FLASH addresses overlaps currently running code
bool
flash_code_overlaps(void *start, void *end)
{
    extern char _stext[], _etext[];

    return ((((unsigned long)&_stext >= (unsigned long)start) &&
             ((unsigned long)&_stext < (unsigned long)end)) ||
            (((unsigned long)&_etext >= (unsigned long)start) &&
             ((unsigned long)&_etext < (unsigned long)end)));
}

// EOF synth.c
