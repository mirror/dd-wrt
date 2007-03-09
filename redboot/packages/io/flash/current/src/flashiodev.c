//==========================================================================
//
//      flashiodev.c
//
//      Flash device interface to I/O subsystem
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
// Author(s):    jlarmour
// Contributors: woehler
// Date:         2002-01-16
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#define _FLASH_PRIVATE_
#include <pkgconf/io_flash.h>

#include <errno.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/io/devtab.h>
#include <cyg/io/config_keys.h>
#include <cyg/io/flash.h>
#include <string.h> // memcpy
#include <cyg/hal/hal_if.h>

#define MIN(x,y) ((x)<(y) ? (x) : (y))

struct flashiodev_priv_t{
	char *start;
	char *end;
};

static struct flashiodev_priv_t flashiodev_priv[1];

static int dummy_printf( const char *fmt, ... ) {return 0;}

static bool
flashiodev_init( struct cyg_devtab_entry *tab )
{
    struct flashiodev_priv_t *dev = (struct flashiodev_priv_t *)tab->priv;
    int stat = flash_init( &dummy_printf );
    if ( stat == 0 ) {
#ifdef CYGNUM_IO_FLASH_BLOCK_CFG_FIS_1
        CYG_ADDRESS		flash_base;
        unsigned long	size;

        if(!CYGACC_CALL_IF_FLASH_FIS_OP(CYGNUM_CALL_IF_FLASH_FIS_GET_FLASH_BASE, 
                                        CYGDAT_IO_FLASH_BLOCK_FIS_NAME_1,
                                        &flash_base))
            return false;
        if(!CYGACC_CALL_IF_FLASH_FIS_OP(CYGNUM_CALL_IF_FLASH_FIS_GET_SIZE, 
                                        CYGDAT_IO_FLASH_BLOCK_FIS_NAME_1,
                                        &size))
            return false;
			
        dev->start = (char *)flash_base;
        dev->end = (char *)flash_base + size;
#else
        dev->start = (char *)flash_info.start + CYGNUM_IO_FLASH_BLOCK_OFFSET_1;
        dev->end = (char *)flash_info.start + CYGNUM_IO_FLASH_BLOCK_OFFSET_1 + 
            CYGNUM_IO_FLASH_BLOCK_LENGTH_1;
#endif
        return true;
    } else
        return false;
} // flashiodev_init()

#if 0
static Cyg_ErrNo
flashiodev_lookup( struct cyg_devtab_entry **tab, 
                   struct cyg_devtab_entry *sub_tab,
                   const char *name)
{   
} // flashiodev_lookup()
#endif

static Cyg_ErrNo
flashiodev_bread( cyg_io_handle_t handle, void *buf, cyg_uint32 *len,
                  cyg_uint32 pos)
{
	struct cyg_devtab_entry *tab = (struct cyg_devtab_entry *)handle;
	struct flashiodev_priv_t *dev = (struct flashiodev_priv_t *)tab->priv;

	char *startpos = dev->start + pos;
        void *erraddr;
        Cyg_ErrNo err = ENOERR;
        

#ifdef CYGPKG_INFRA_DEBUG // don't bother checking this all the time
    char *endpos = startpos + *len - 1;
    char *flashend = MIN( (char *)flash_info.end - 1, dev->end - 1);
    if ( startpos < dev->start )
        return -EINVAL;
    if ( endpos > flashend )
        return -EINVAL;
#endif
    
    err = flash_read( startpos,
                      (void *)buf, *len, &erraddr );

    if ( err )
        err = -EIO; // just something sane
    return err;
} // flashiodev_bread()

static Cyg_ErrNo
flashiodev_bwrite( cyg_io_handle_t handle, const void *buf, cyg_uint32 *len,
                   cyg_uint32 pos )
{   
	struct cyg_devtab_entry *tab = (struct cyg_devtab_entry *)handle;
	struct flashiodev_priv_t *dev = (struct flashiodev_priv_t *)tab->priv;

    Cyg_ErrNo err = ENOERR;
    void *erraddr;
    char *startpos = dev->start + pos;

#ifdef CYGPKG_INFRA_DEBUG // don't bother checking this all the time
    char *endpos = startpos + *len - 1;
    char *flashend = MIN( (char *)flash_info.end - 1, dev->end - 1);
    if ( startpos < dev->start )
        return -EINVAL;
    if ( endpos > flashend )
        return -EINVAL;
#endif
    err = flash_program( startpos, 
                         (void *)buf, *len, &erraddr );

    if ( err )
        err = -EIO; // just something sane
    return err;
} // flashiodev_bwrite()

static Cyg_ErrNo
flashiodev_get_config( cyg_io_handle_t handle,
                       cyg_uint32 key,
                       void* buf,
                       cyg_uint32* len)
{
	struct cyg_devtab_entry *tab = (struct cyg_devtab_entry *)handle;
	struct flashiodev_priv_t *dev = (struct flashiodev_priv_t *)tab->priv;

    switch (key) {
    case CYG_IO_GET_CONFIG_FLASH_ERASE:
    {
        if ( *len != sizeof( cyg_io_flash_getconfig_erase_t ) )
             return -EINVAL;
        {
            cyg_io_flash_getconfig_erase_t *e = (cyg_io_flash_getconfig_erase_t *)buf;
            char *startpos = dev->start + e->offset;

#ifdef CYGPKG_INFRA_DEBUG // don't bother checking this all the time
            char *endpos = startpos + e->len - 1;
            char *flashend = MIN( (char *)flash_info.end - 1, dev->end - 1);
            if ( startpos < dev->start )
                return -EINVAL;
            if ( endpos > flashend )
                return -EINVAL;
#endif
            e->flasherr = flash_erase( startpos, e->len, e->err_address );
        }
        return ENOERR;
    }
    case CYG_IO_GET_CONFIG_FLASH_DEVSIZE:
    {
        if ( *len != sizeof( cyg_io_flash_getconfig_devsize_t ) )
             return -EINVAL;
        {
            cyg_io_flash_getconfig_devsize_t *d =
                (cyg_io_flash_getconfig_devsize_t *)buf;

	    //d->dev_size = flash_info.blocks * flash_info.block_size;
			d->dev_size = dev->end - dev->start;
        }
        return ENOERR;
    }

    case CYG_IO_GET_CONFIG_FLASH_BLOCKSIZE:
    {
        cyg_io_flash_getconfig_blocksize_t *b =
            (cyg_io_flash_getconfig_blocksize_t *)buf;
#ifdef CYGPKG_INFRA_DEBUG // don't bother checking this all the time
       char *startpos = dev->start + b->offset;
	    char *flashend = MIN( (char *)flash_info.end - 1, dev->end - 1);

        if ( startpos < dev->start )
            return -EINVAL;
        if ( startpos > flashend )
            return -EINVAL;
#endif  
        if ( *len != sizeof( cyg_io_flash_getconfig_blocksize_t ) )
             return -EINVAL;
        
        // offset unused for now
		b->block_size = flash_info.block_size;
        return ENOERR;
    }

    default:
        return -EINVAL;
    }
} // flashiodev_get_config()

static Cyg_ErrNo
flashiodev_set_config( cyg_io_handle_t handle,
                       cyg_uint32 key,
                       const void* buf,
                       cyg_uint32* len)
{
#ifdef CYGNUM_IO_FLASH_BLOCK_CFG_FIS_1
	struct cyg_devtab_entry *tab = (struct cyg_devtab_entry *)handle;
	struct flashiodev_priv_t *dev = (struct flashiodev_priv_t *)tab->priv;
#endif

    switch (key) {
#ifdef CYGNUM_IO_FLASH_BLOCK_CFG_FIS_1
    case CYG_IO_SET_CONFIG_FLASH_FIS_NAME:
    {
        CYG_ADDRESS     flash_base;
	unsigned long   size;

	if(!CYGACC_CALL_IF_FLASH_FIS_OP(CYGNUM_CALL_IF_FLASH_FIS_GET_FLASH_BASE, 
                                        (char *)buf, &flash_base))
	    return -ENOENT;
	if(!CYGACC_CALL_IF_FLASH_FIS_OP(CYGNUM_CALL_IF_FLASH_FIS_GET_SIZE, 
					(char *)buf, &size))
	    return -ENOENT;
			
	dev->start = (char *)flash_base;
	dev->end = (char *)flash_base + size;
	return ENOERR;
    }
#endif
    default:
        return -EINVAL;
    }
} // flashiodev_set_config()

// get_config/set_config should be added later to provide the other flash
// operations possible, like erase etc.

BLOCK_DEVIO_TABLE( cyg_io_flashdev1_ops,
                   &flashiodev_bwrite,
                   &flashiodev_bread,
                   0, // no select
                   &flashiodev_get_config,
                   &flashiodev_set_config
    ); 
                   

BLOCK_DEVTAB_ENTRY( cyg_io_flashdev1,
                    CYGDAT_IO_FLASH_BLOCK_DEVICE_NAME_1,
                    0,
                    &cyg_io_flashdev1_ops,
                    &flashiodev_init,
                    0, // No lookup required
                    &flashiodev_priv[0] );

// EOF flashiodev.c
