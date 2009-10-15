#ifndef CYGONCE_DEVS_FLASH_AR7100_INL
#define CYGONCE_DEVS_FLASH_AR7100_INL
//==========================================================================
//
//      devs_flash_ar7100.inl
//
//      AR7100 SPI FLASH driver
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
// Author(s):    
// Contributors:
// Date:         2003-04-21
// Purpose:
// Description:  AR7100 SPI flash driver
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
//#include <pkgconf/devs_flash_sst_39vf400.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_diag.h> /* HAL_DELAY_US */
#include <cyg/infra/diag.h>   /* Required for diag_printf */
#include CYGHWR_MEMORY_LAYOUT_H

#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>

//----------------------------------------------------------------------------
// Platform code must define the below
// #define CYGNUM_FLASH_BLOCK_NUM         : Number of flash blocks
// #define CYGNUM_FLASH_BLOCK_SIZE        : Size of flash block
// #define CYGNUM_FLASH_BASE              : Base address of the FLASH
// #define CYGNUM_FLASH_END_RESERVED_BYTES: Reserved bytes for platform info
#define CYGNUM_FLASH_BLANK       (1)
// Now that device properties are defined, include magic for defining
// accessor type and constants.
#include <cyg/io/flash_dev.h>

#include <cyg/hal/ar7100_soc.h>
#include <cyg/io/ar7100_flash.h>

//----------------------------------------------------------------------------
// Functions that put the flash device into non-read mode must reside
// in RAM.
void flash_query(void* data) __attribute__ ((section (".2ram.flash_query")));
int  flash_erase_block(void* block, unsigned int size)
    __attribute__ ((section (".2ram.flash_erase_block")));
int  flash_program_buf(void* addr, void* data, int len)
    __attribute__ ((section (".2ram.flash_program_buf")));


/*
 * statics
 */
static void ar7100_spi_write_enable(void)
      __attribute__((section (".2ram.ar7100_spi_write_enable")));
static void ar7100_spi_poll(void)
      __attribute__((section (".2ram.ar7100_spi_poll")));
static void ar7100_spi_write_page(cyg_uint32 addr, cyg_uint8 *data, int len)
      __attribute__((section (".2ram.ar7100_spi_write_page")));
static void ar7100_spi_sector_erase(cyg_uint32 addr)
      __attribute__((section (".2ram.ar7100_spi_sector_erase")));

#if 0
static void
read_id()
{
    cyg_uint32 rd = 0x777777;

    ar7100_reg_wr_nf(AR7100_SPI_WRITE, AR7100_SPI_CS_DIS);
    ar7100_spi_bit_banger(0x9f);
    ar7100_spi_delay_8();
    ar7100_spi_delay_8();
    ar7100_spi_delay_8();
    ar7100_spi_done();
    /* rd = ar7100_reg_rd(AR7100_SPI_RD_STATUS); */
    ar7100_reg_wr_nf(AR7100_SPI_FS, 1);
    rd = ar7100_reg_rd(AR7100_SPI_READ); 
    ar7100_reg_wr_nf(AR7100_SPI_FS, 0);
    diag_printf("id read %x\n", rd);
}
#endif

int
flash_hwr_init (void)
{
    int i;

    ar7100_reg_wr_nf(AR7100_SPI_CLOCK, 0x43);
    //read_id();

    flash_info.blocks = CYGNUM_FLASH_BLOCK_NUM;

    flash_info.block_size = CYGNUM_FLASH_BLOCK_SIZE;
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
int flash_erase_block(void* block, unsigned int size)
{
    int i = (cyg_uint32)block/CYGNUM_FLASH_BLOCK_SIZE, s_last = size/CYGNUM_FLASH_BLOCK_SIZE;

    //diag_printf("First %x last %x\n", i, s_last);

    do {
        //diag_printf("erasing flash sect %d\n", i);
        ar7100_spi_sector_erase(i * CYGNUM_FLASH_BLOCK_SIZE);
    }while (++i < s_last);

    ar7100_spi_done();

    return 0;
}

//----------------------------------------------------------------------------
// Program Buffer
// 0. Assumption: Caller has already erased the appropriate sectors.
// 1. call page programming for every 256 bytes

int
flash_program_buf(void* addr, void* data, int len)
{
    int total = 0, len_this_lp, bytes_this_page;
    unsigned long dst;
    unsigned char *src;
    unsigned long uladdr = (unsigned long)addr - CYGNUM_FLASH_BASE;

    while(total < len) {
        src              = (unsigned char*)data + total;
        dst              = uladdr + total;
        bytes_this_page  = AR7100_SPI_PAGE_SIZE - ((unsigned long)uladdr % AR7100_SPI_PAGE_SIZE);
        len_this_lp      = ((len - total) > bytes_this_page) ? bytes_this_page
                                                             : (len - total);

        ar7100_spi_write_page(dst, src, len_this_lp);
        total += len_this_lp;
    }

    ar7100_spi_done();

    return 0;
}

static void
ar7100_spi_write_enable()  
{
    ar7100_reg_wr_nf(AR7100_SPI_FS, 1);                  
    ar7100_reg_wr_nf(AR7100_SPI_WRITE, AR7100_SPI_CS_DIS);     
    ar7100_spi_bit_banger(AR7100_SPI_CMD_WREN);             
    ar7100_spi_go();
}

static void
ar7100_spi_poll()   
{
    int rd;                                                 

    do {
        ar7100_reg_wr_nf(AR7100_SPI_WRITE, AR7100_SPI_CS_DIS);     
        ar7100_spi_bit_banger(AR7100_SPI_CMD_RD_STATUS);        
        ar7100_spi_delay_8();
        rd = (ar7100_reg_rd(AR7100_SPI_RD_STATUS) & 1);               
    }while(rd);
}

static void
ar7100_spi_write_page(cyg_uint32 addr, cyg_uint8 *data, int len)
{
    int i;
    cyg_uint8 ch;

    display(0x77);
    ar7100_spi_write_enable();
    ar7100_spi_bit_banger(AR7100_SPI_CMD_PAGE_PROG);
    ar7100_spi_send_addr(addr);

    for(i = 0; i < len; i++) {
        ch = *(data + i);
        ar7100_spi_bit_banger(ch);
    }

    ar7100_spi_go();
    display(0x66);
    ar7100_spi_poll();
    display(0x6d);
}

static void
ar7100_spi_sector_erase(cyg_uint32 addr)
{
    ar7100_spi_write_enable();
    ar7100_spi_bit_banger(AR7100_SPI_CMD_SECTOR_ERASE);
    ar7100_spi_send_addr(addr);
    ar7100_spi_go();
    display(0x7d);
    ar7100_spi_poll();
}

#endif // CYGONCE_DEVS_FLASH_AR7100_INL







