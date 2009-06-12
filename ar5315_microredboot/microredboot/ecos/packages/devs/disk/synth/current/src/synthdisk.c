//==========================================================================
//
//      synthdisk.c
//
//      Disk device driver for the synthetic target 
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Savin Zlobec.
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    savin
// Contributors: 
// Date:         2003-06-18
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/devs_disk_ecosynth.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/io/disk.h>

#include <stdio.h> // sprintf

// ----------------------------------------------------------------------------

//#define DEBUG 1

// ----------------------------------------------------------------------------

typedef struct {
    int        num;
    cyg_uint32 cylinders_num;
    cyg_uint32 heads_num;
    cyg_uint32 sectors_num;
    cyg_uint32 size;
    int        filefd;
    char       *filename;
} synth_disk_info_t;

// ----------------------------------------------------------------------------

static cyg_bool synth_disk_init(struct cyg_devtab_entry *tab);

static Cyg_ErrNo synth_disk_read(disk_channel *chan, 
                                 void         *buf,
                                 cyg_uint32    len,
                                 cyg_uint32    block_num);

static Cyg_ErrNo synth_disk_write(disk_channel *chan, 
                                  const void   *buf,
                                  cyg_uint32    len,
                                  cyg_uint32    block_num);

static Cyg_ErrNo synth_disk_get_config(disk_channel *chan, 
                                       cyg_uint32    key,
                                       const void   *xbuf, 
                                       cyg_uint32   *len);

static Cyg_ErrNo synth_disk_set_config(disk_channel *chan, 
                                       cyg_uint32    key,
                                       const void   *xbuf, 
                                       cyg_uint32   *len);

static Cyg_ErrNo synth_disk_lookup(struct cyg_devtab_entry  **tab,
                                   struct cyg_devtab_entry   *sub_tab,
                                   const char                *name);

static DISK_FUNS(synth_disk_funs, 
                 synth_disk_read, 
                 synth_disk_write, 
                 synth_disk_get_config,
                 synth_disk_set_config
);

// ----------------------------------------------------------------------------

#define SYNTH_DISK_INSTANCE(_number_,_mbr_supp_, _cyl_,_hpt_,_spt_)        \
static synth_disk_info_t synth_disk_info##_number_ = {                     \
    num:           _number_,                                               \
    cylinders_num: _cyl_,                                                  \
    heads_num:     _hpt_,                                                  \
    sectors_num:   _spt_,                                                  \
    size:          CYGNUM_IO_DISK_ECOSYNTH_DISK##_number_##_SIZE,          \
    filefd:        -1,                                                     \
    filename:      CYGDAT_IO_DISK_ECOSYNTH_DISK##_number_##_FILENAME       \
};                                                                         \
DISK_CHANNEL(synth_disk_channel##_number_,                                 \
             synth_disk_funs,                                              \
             synth_disk_info##_number_,                                    \
             _mbr_supp_,                                                   \
             4                                                             \
);                                                                         \
BLOCK_DEVTAB_ENTRY(synth_disk_io##_number_,                                \
             CYGDAT_IO_DISK_ECOSYNTH_DISK##_number_##_NAME,                \
             0,                                                            \
             &cyg_io_disk_devio,                                           \
             synth_disk_init,                                              \
             synth_disk_lookup,                                            \
             &synth_disk_channel##_number_                                 \
);

// ----------------------------------------------------------------------------

#ifdef CYGVAR_DEVS_DISK_ECOSYNTH_DISK0
# ifndef CYGIMP_IO_DISK_ECOSYNTH_DISK0_MBR
SYNTH_DISK_INSTANCE(0, false, 0, 0, 0);
# else
SYNTH_DISK_INSTANCE(0, true, CYGIMP_IO_DISK_ECOSYNTH_DISK0_CYLINDERS,
                             CYGIMP_IO_DISK_ECOSYNTH_DISK0_HEADS,
                             CYGIMP_IO_DISK_ECOSYNTH_DISK0_SECTORS);
# endif
#endif

// ----------------------------------------------------------------------------

static cyg_bool 
synth_disk_init(struct cyg_devtab_entry *tab)
{
    disk_channel      *chan       = (disk_channel *) tab->priv;
    synth_disk_info_t *synth_info = (synth_disk_info_t *) chan->dev_priv;
    bool result = true;

    if (chan->init) 
        return true;

#ifdef DEBUG
    diag_printf("synth disk %d init size=%d\n", 
                synth_info->num, synth_info->size);
#endif
   
    synth_info->filefd = cyg_hal_sys_open(synth_info->filename, 
            CYG_HAL_SYS_O_RDWR,
            CYG_HAL_SYS_S_IRWXU|CYG_HAL_SYS_S_IRWXG|CYG_HAL_SYS_S_IRWXO);

    if (-ENOENT == synth_info->filefd)
    {
        synth_info->filefd = cyg_hal_sys_open(synth_info->filename, 
            CYG_HAL_SYS_O_RDWR|CYG_HAL_SYS_O_CREAT, 0644);
        
        if (synth_info->filefd >= 0)
        {
            unsigned char b = 0x00;
            int i;
        
            for (i = 0; i < synth_info->size; i++)
                cyg_hal_sys_write(synth_info->filefd, &b, 1);    
        }
    }

    if (synth_info->filefd < 0)
    {
        CYG_ASSERT(false, "Can't open/create disk image file");
        return false;
    }
   
    if (result)
    {
        cyg_disk_identify_t ident;
       
        ident.serial[0]       = '\0';
        ident.firmware_rev[0] = '\0';
        ident.model_num[0]    = '\0';
        ident.lba_sectors_num = synth_info->size / 512;
        ident.cylinders_num   = synth_info->cylinders_num;
        ident.heads_num       = synth_info->heads_num;
        ident.sectors_num     = synth_info->sectors_num;
 
        if (!(chan->callbacks->disk_init)(tab))
            return false;
        if (ENOERR != (chan->callbacks->disk_connected)(tab, &ident))
            return false;
    }
    return result;
}

static Cyg_ErrNo 
synth_disk_lookup(struct cyg_devtab_entry **tab, 
                  struct cyg_devtab_entry  *sub_tab,
                  const char               *name)
{
    disk_channel *chan = (disk_channel *) (*tab)->priv;
    return (chan->callbacks->disk_lookup(tab, sub_tab, name));
}

static Cyg_ErrNo 
synth_disk_read(disk_channel *chan, 
                void         *buf,
                cyg_uint32    len,
                cyg_uint32    block_num)
{
    synth_disk_info_t *synth_info = (synth_disk_info_t *)chan->dev_priv;

#ifdef DEBUG
    diag_printf("synth disk read block %d\n", block_num);
#endif
    
    if (synth_info->filefd >= 0)
    {
        cyg_hal_sys_lseek(synth_info->filefd, 
                          block_num * chan->info->block_size,
                          CYG_HAL_SYS_SEEK_SET);
        cyg_hal_sys_read(synth_info->filefd, buf, len);
        return ENOERR;
    }
    return -EIO; 
}

static Cyg_ErrNo 
synth_disk_write(disk_channel *chan,
                 const void   *buf,
                 cyg_uint32    len,
                 cyg_uint32    block_num)
{
    synth_disk_info_t *synth_info = (synth_disk_info_t *)chan->dev_priv;

#ifdef DEBUG
    diag_printf("synth disk write block %d\n", block_num);
#endif
 
    if (synth_info->filefd >= 0)
    {
        cyg_hal_sys_lseek(synth_info->filefd, 
                          block_num * chan->info->block_size,
                          CYG_HAL_SYS_SEEK_SET);
        cyg_hal_sys_write(synth_info->filefd, buf, len);
//        cyg_hal_sys_fdatasync(synth_info->filefd);
        return ENOERR;
    }
    return -EIO; 
}

static Cyg_ErrNo
synth_disk_get_config(disk_channel *chan, 
                      cyg_uint32    key,
                      const void   *xbuf, 
                      cyg_uint32   *len)
{

#ifdef DEBUG
    diag_printf("synth disk get config\n");
#endif
    
    return -EINVAL;
}

static Cyg_ErrNo
synth_disk_set_config(disk_channel *chan, 
                      cyg_uint32    key,
                      const void   *xbuf, 
                      cyg_uint32   *len)
{

#ifdef DEBUG
    diag_printf("synth disk set config\n");
#endif
 
    return -EINVAL;
}

// ----------------------------------------------------------------------------
// EOF synthdisk.c
