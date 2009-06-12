//==========================================================================
//
//      v85x_edb_v850_disk.c
//
//      Elatec v850 development board disk driver 
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
// Date:         2003-08-21
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/devs_disk_v85x_edb_v850.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/io/disk.h>

#include <cf_ata.h>

// ----------------------------------------------------------------------------

//#define DEBUG 1

#ifdef DEBUG
# define D(_args_) diag_printf _args_
#else
# define D(_args_)
#endif

// ----------------------------------------------------------------------------

#include <cyg/hal/v850_common.h>

static volatile unsigned char* P10  = (volatile unsigned char*)V850_REG_P10;
static volatile unsigned char* PM10 = (volatile unsigned char*)V850_REG_PM10;
static volatile unsigned char* PU10 = (volatile unsigned char*)V850_REG_PU10;

#define CF_BASE 0x00380000

#define CF_HW_INIT()                  \
    do {                              \
        *PU10 |= (1<<4);              \
        *PM10 |= (1<<4);              \
        *PM10 &= ~2;                  \
    } while (0)

#define CF_HW_RESET()                   \
    do {                                \
        int i;                          \
        *P10 |=  2;                     \
        for (i = 0; i < 500000; i++);   \
        *P10 &= ~2;                     \
        for (i = 0; i < 500000; i++);   \
    } while (0)

#define CF_HW_BUSY_WAIT()               \
    do {                                \
        while (0 == (*P10 & (1<<4)));   \
    } while (0)

// ----------------------------------------------------------------------------

typedef struct {
    volatile cyg_uint16 *base;
} cf_disk_info_t;

// ----------------------------------------------------------------------------

static cyg_bool cf_disk_init(struct cyg_devtab_entry *tab);

static Cyg_ErrNo cf_disk_read(disk_channel *chan, 
                              void         *buf,
                              cyg_uint32    len, 
                              cyg_uint32    block_num); 
        

static Cyg_ErrNo cf_disk_write(disk_channel *chan, 
                               const void   *buf,
                               cyg_uint32    len, 
                               cyg_uint32    block_num); 
 
static Cyg_ErrNo cf_disk_get_config(disk_channel *chan, 
                                    cyg_uint32    key,
                                    const void   *xbuf, 
                                    cyg_uint32   *len);

static Cyg_ErrNo cf_disk_set_config(disk_channel *chan, 
                                    cyg_uint32    key,
                                    const void   *xbuf, 
                                    cyg_uint32   *len);

static Cyg_ErrNo cf_disk_lookup(struct cyg_devtab_entry **tab,
                                struct cyg_devtab_entry  *sub_tab,
                                const char               *name);

static DISK_FUNS(cf_disk_funs, 
                 cf_disk_read, 
                 cf_disk_write, 
                 cf_disk_get_config,
                 cf_disk_set_config
);

// ----------------------------------------------------------------------------

#define CF_DISK_INSTANCE(_number_,_base_,_mbr_supp_,_name_)           \
static cf_disk_info_t cf_disk_info##_number_ = {                      \
    base: (volatile cyg_uint16 *)_base_,                              \
};                                                                    \
DISK_CHANNEL(cf_disk_channel##_number_,                               \
             cf_disk_funs,                                            \
             cf_disk_info##_number_,                                  \
             _mbr_supp_,                                              \
             4                                                        \
);                                                                    \
BLOCK_DEVTAB_ENTRY(cf_disk_io##_number_,                              \
                   _name_,                                            \
                   0,                                                 \
                   &cyg_io_disk_devio,                                \
                   cf_disk_init,                                      \
                   cf_disk_lookup,                                    \
                   &cf_disk_channel##_number_                         \
);

// ----------------------------------------------------------------------------

#ifdef CYGVAR_DEVS_DISK_V85X_EDB_V850_DISK0
CF_DISK_INSTANCE(0, CF_BASE, true, CYGDAT_IO_DISK_V85X_EDB_V850_DISK0_NAME);
#endif

// ----------------------------------------------------------------------------

static __inline__ cyg_uint8
get_status(volatile cyg_uint16 *base)
{
    cyg_uint16 val;
    HAL_READ_UINT16(base + 7, val);
    return (val & 0x00FF);
}

static __inline__ cyg_uint8
get_error(volatile cyg_uint16 *base)
{
    cyg_uint16 val;
    HAL_READ_UINT16(base + 6, val);
    return ((val >> 8) & 0x00FF);
}

static __inline__ void
set_dctrl(volatile cyg_uint16 *base, cyg_uint8 dbits)
{
    cyg_uint16 val = dbits;
    HAL_WRITE_UINT16(base + 7, val);
}

static cyg_bool 
exe_cmd(volatile cyg_uint16 *base,
        cyg_uint8            cmd,
        cyg_uint8            drive,
        cyg_uint32           lba_addr,
        cyg_uint8            sec_cnt)
{
    cyg_uint8  lba0_7;
    cyg_uint16 lba8_23;
    cyg_uint8  lba24_27;
    cyg_uint8  drv;

    lba0_7   = lba_addr & 0xFF;
    lba8_23  = (lba_addr >> 8)  & 0xFFFF;
    lba24_27 = (lba_addr >> 24) & 0x0F;

    // drive and LBA addressing mode flag
    if (0 == drive) drv = 0x40;
    else            drv = 0x50;
     
    CF_HW_BUSY_WAIT();

    HAL_WRITE_UINT16(base + 1, sec_cnt | (lba0_7 << 8));
    HAL_WRITE_UINT16(base + 2, lba8_23);
    HAL_WRITE_UINT16(base + 3, lba24_27 | drv | (cmd << 8));

    CF_HW_BUSY_WAIT();
    
    return (0 == (get_status(base) & CF_SREG_ERR));
}

static void
read_data(volatile cyg_uint16 *base,
          void                *buf,
          cyg_uint16           len)
{
    cyg_uint16 *bufp = (cyg_uint16 *)buf;
    int i;

    CF_HW_BUSY_WAIT();

    for (i = 0; i < 512; i += 2)
    {
        cyg_uint16 data;
        HAL_READ_UINT16(base + 4, data);
        if (i < len) *bufp++ = data;
    }

    CF_HW_BUSY_WAIT();
}

static void
write_data(volatile cyg_uint16 *base,
           const void          *buf,
           cyg_uint16           len)
{
    cyg_uint16 *bufp = (cyg_uint16 * const)buf;
    int i;

    CF_HW_BUSY_WAIT();

    for (i = 0; i < 512; i += 2)
    {
        cyg_uint16 data;

        if (i < len) data = *bufp++;
        else         data = 0x0000;

        HAL_WRITE_UINT16(base + 4, data);
    }
    
    CF_HW_BUSY_WAIT();
}

static void
id_strcpy(char *dest, cyg_uint16 *src, cyg_uint16 size)
{
    int i;

    for (i = 0; i < size; i+=2)
    {
        *dest++ = (char)(*src >> 8);
        *dest++ = (char)(*src & 0x00FF);
        src++;
    }
    *dest = '\0';
}

// ----------------------------------------------------------------------------

static cyg_bool 
cf_disk_init(struct cyg_devtab_entry *tab)
{
    disk_channel           *chan = (disk_channel *) tab->priv;
    cf_disk_info_t         *info = (cf_disk_info_t *) chan->dev_priv;
    cf_ata_identify_data_t *ata_id;
    cyg_uint8               id_buf[sizeof(cf_ata_identify_data_t)];
    cyg_disk_identify_t     ident;
    
    if (chan->init) 
        return true;

    D(("CF(%p) hw init\n", info->base));

    CF_HW_INIT();
    CF_HW_RESET();
    
    D(("CF(%p) identify drive\n", info->base));

    if (!exe_cmd(info->base, CF_ATA_IDENTIFY_DRIVE_CMD, 0, 0, 0))
    {
        D(("CF(%p) error (%x)\n", info->base, get_error(info->base)));
        return false; 
    }
    read_data(info->base, id_buf, sizeof(cf_ata_identify_data_t));
   
    ata_id = (cf_ata_identify_data_t *)id_buf;

    D(("CF(%p) general conf = %x\n", info->base, ata_id->general_conf));
            
    if (0x848A != ata_id->general_conf)
        return false;
    
    id_strcpy(ident.serial,       ata_id->serial,       20);
    id_strcpy(ident.firmware_rev, ata_id->firmware_rev, 8);
    id_strcpy(ident.model_num,    ata_id->model_num,    40);
    
    ident.cylinders_num   = ata_id->num_cylinders;
    ident.heads_num       = ata_id->num_heads;
    ident.sectors_num     = ata_id->num_sectors;
    ident.lba_sectors_num = ata_id->lba_total_sectors[1] << 16 | 
                            ata_id->lba_total_sectors[0];
    
    if (!(chan->callbacks->disk_init)(tab))
        return false;

    if (ENOERR != (chan->callbacks->disk_connected)(tab, &ident))
        return false;

    return true;
}

static Cyg_ErrNo 
cf_disk_lookup(struct cyg_devtab_entry **tab, 
               struct cyg_devtab_entry  *sub_tab,
               const char *name)
{
    disk_channel *chan = (disk_channel *) (*tab)->priv;
    return (chan->callbacks->disk_lookup)(tab, sub_tab, name);
}

static Cyg_ErrNo 
cf_disk_read(disk_channel *chan, 
             void         *buf,
             cyg_uint32    len, 
             cyg_uint32    block_num)
{
    cf_disk_info_t *info = (cf_disk_info_t *)chan->dev_priv;

    D(("CF(%p) read block %d\n", info->base, block_num));

    if (!exe_cmd(info->base, CF_ATA_READ_SECTORS_CMD, 0, block_num, 1))
    {
        D(("CF(%p) error (%x)\n", info->base, get_error(info->base)));
        return -EIO; 
    }
    read_data(info->base, buf, len);
        
    return ENOERR;
}

static Cyg_ErrNo 
cf_disk_write(disk_channel *chan, 
              const void   *buf,
              cyg_uint32    len, 
              cyg_uint32    block_num)
{
    cf_disk_info_t *info = (cf_disk_info_t *)chan->dev_priv;

    D(("CF(%p) write block %d\n", info->base, block_num));
 
    if (!exe_cmd(info->base, CF_ATA_WRITE_SECTORS_CMD, 0, block_num, 1))
    {
        D(("CF(%p) error (%x)\n", info->base, get_error(info->base)));
        return -EIO; 
    }
    write_data(info->base, buf, len);
        
    return ENOERR;
}

static Cyg_ErrNo
cf_disk_get_config(disk_channel *chan, 
                   cyg_uint32    key,
                   const void   *xbuf, 
                   cyg_uint32   *len)
{
    return -EINVAL;
}

static Cyg_ErrNo
cf_disk_set_config(disk_channel *chan, 
                   cyg_uint32    key,
                   const void   *xbuf, 
                   cyg_uint32   *len)
{
    return -EINVAL;
}

// ----------------------------------------------------------------------------
// EOF v85x_edb_v850_disk.c
