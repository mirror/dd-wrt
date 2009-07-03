//==========================================================================
//
//      spiflash.c
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
// Author(s):    gthomas, hmt
// Contributors: gthomas
// Date:         2001-02-14
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>
#define _FLASH_PRIVATE_
#include <cyg/io/flash.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_io.h>

#include "spiflash.h"

static cyg_uint32
sfi_Command(cyg_uint32 opcode, int write_len, int read_len)
{
    cyg_uint32 reg, mask;

    do {
        HAL_READ_UINT32(AR2316_SPI_CTL, reg);
    } while (reg & SPI_CTL_BUSY);
    HAL_WRITE_UINT32(AR2316_SPI_OPCODE, opcode);
    reg = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | (write_len<<0) | (read_len<<4) | SPI_CTL_START;
    HAL_WRITE_UINT32(AR2316_SPI_CTL,reg);
    if (read_len > 0) {
        do {
            HAL_READ_UINT32(AR2316_SPI_CTL, reg);
        } while (reg & SPI_CTL_BUSY);
        HAL_READ_UINT32(AR2316_SPI_DATA, reg);
        switch (read_len) {
        case 1:
            mask = 0x000000ff;
            break;
        case 2:
            mask = 0x0000ffff;
            break;
        case 3:
            mask = 0x00ffffff;
            break;
        default:
            mask = 0xffffffff;
            break;
        }
        reg &= mask;
    } else {
        reg = 0;
    }
    return reg;
}

/*
 * List of supported flash devices. The first index is the
 * flash id returned by the probe.  The second is the number
 * of blocks. The list is terminated with an id of -1
 */
static cyg_int32 suppFlashList[][2] = {
    {0x13, 16},
    {0x14, 32},
    {0x15, 64},
    {0x16, 128},
    {-1, -1}};

static cyg_int32
get_num_blocks(cyg_uint32 id)
{
    cyg_int32 cnt=0;

    while (suppFlashList[cnt][0] != -1) {
	if (suppFlashList[cnt][0] == id) {
	    return(suppFlashList[cnt][1]);
	}
	cnt++;
    }
    return 0;
}

int
flash_hwr_init(void)
{
    cyg_uint32 id;

    id = sfi_Command(STM_OP_RD_SIG, 4, 1);

    flash_info.buffer_size = 0;
    flash_info.block_size = CYGNUM_FLASH_BLOCK_SIZE;
    flash_info.blocks = get_num_blocks(id);
    if (flash_info.blocks == 0) {
	diag_printf("%s: Unsupported flash device - id=%d\n",__func__,id);
	return FLASH_ERR_DRV_WRONG_PART;
    }

    flash_info.start = (void *)CYGNUM_FLASH_BASE;
    flash_info.end = (void *)( CYGNUM_FLASH_BASE + (flash_info.block_size * flash_info.blocks));
#ifdef CYGNUM_FLASH_END_RESERVED_BYTES
    flash_info.end = (void *)((unsigned int) flash_info.end - CYGNUM_FLASH_END_RESERVED_BYTES);
#endif
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

void udelay(int usec);

int flash_erase_block(volatile flash_t *block, unsigned int block_size)
	__attribute__ ((section (".2ram.flash_erase_block")));
int flash_erase_block(volatile flash_t *block, unsigned int block_size)
{
    cyg_uint32 res;
    cyg_uint32 offset = (cyg_uint32)block -  CYGNUM_FLASH_BASE;

    sfi_Command(STM_OP_WR_ENABLE, 1, 0);
	do {
		res = sfi_Command(STM_OP_RD_STATUS, 1, 1);
		if ((res & 0x3)==0x2) {
			break;
		}
		udelay(20);
		sfi_Command(STM_OP_WR_ENABLE, 1, 0);
	} while (1);
    sfi_Command(STM_OP_SECTOR_ERASE | (offset << 8), 4, 0);
    while (true) {
        res = sfi_Command(STM_OP_RD_STATUS, 1, 1);
        if ((res & STM_STATUS_WIP) == 0) {
            break;
        }
    }
    return FLASH_ERR_OK;
}

int
flash_program_buf(volatile flash_t *addr, flash_t *buf, int len,
                  unsigned long block_mask, int buffer_size)
 __attribute__ ((section (".2ram.flash_program_buf")));
 
/*GPIO0 based page programming support, stoneshih, 20Aug2007*/
#define AP61_R02HW 1
#ifdef AP61_R02HW 

#define STM_PAGE_SIZE 256
#define PAGE_PROGRAM_OPCODE 2

extern int page_programming_supported;
extern int page_gpio;

static unsigned
spiflash_send_one_data_byte (unsigned char byte_in)  /*for page programming*/
{
	unsigned reg;
	 
	do {
		HAL_READ_UINT32(AR2316_SPI_CTL, reg);
	} while (reg & SPI_CTL_BUSY);
	
	HAL_WRITE_UINT32(AR2316_SPI_OPCODE, byte_in);
	
	reg = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | 1 | SPI_CTL_START;/*send just 1 byte*/
	
	HAL_WRITE_UINT32(AR2316_SPI_CTL,reg);
		
	return reg;
}
void udelay(int usecs) 
{
	unsigned u1, clks;		
	clks = usecs*40;
	u1 = *(volatile unsigned*)0xb1000030;
	while (u1 - *(volatile unsigned*)0xb1000030 < clks);
}

int
flash_program_buf(volatile flash_t *addr, flash_t *buf, int len,
                  unsigned long block_mask, int buffer_size)
{
    cyg_uint32 res;
    cyg_uint32 offset = (cyg_uint32)addr -  CYGNUM_FLASH_BASE;
    cyg_uint32 data = 0;
    int data_len;
    cyg_uint8 *cb = (cyg_uint8 *)buf;
	int page_offset;
	unsigned opcode, reg;
	int first_spi_write_data_length,second_direct_byte_write_data_length,i;

    while (len > 0) {
        sfi_Command(STM_OP_WR_ENABLE, 1, 0);
		do {
			res = sfi_Command(STM_OP_RD_STATUS, 1, 1);
			if ((res & 0x3)==0x2) {
				break;
			}
			udelay(20);
			sfi_Command(STM_OP_WR_ENABLE, 1, 0);
		} while (1);
        
		if (page_programming_supported){
			if (len < STM_PAGE_SIZE)
				data_len = len;
			else
				data_len = STM_PAGE_SIZE;
		} 
		else {
			if (len < 4)
				data_len = len;
			else
				data_len = 4;
		}
		
		page_offset = (offset & (STM_PAGE_SIZE - 1)) + data_len;

		if (page_offset > STM_PAGE_SIZE) {
			data_len -= (page_offset - STM_PAGE_SIZE);
		}

        do {
   	        HAL_READ_UINT32(AR2316_SPI_CTL, reg);
       	} while (reg & SPI_CTL_BUSY);

		if (data_len <= 4) {
	        switch (data_len) {
    	    case 1:
        	    data = *cb;
            	break;
        	case 2:
            	data = (cb[1] << 8) | cb[0];
            	break;
        	case 3:
            	data = (cb[2] << 16) | (cb[1] << 8) | cb[0];
            	break;
        	case 4:
            	data = (cb[3] << 24) | (cb[2] << 16) | (cb[1] << 8) | cb[0];
            	break;
        	}
			first_spi_write_data_length = data_len;
			second_direct_byte_write_data_length = 0;
		}
		else {
			data = (cb[3] << 24) | (cb[2] << 16) | (cb[1] << 8) | cb[0];
			first_spi_write_data_length = 4;
			second_direct_byte_write_data_length = data_len - 4;
		}
        	
		if ((page_programming_supported) || (data != 0xffffffff)) {

			//should disable interrupt here to avoid time gap between writes
			//__asm("di");//disable interrupt

			HAL_WRITE_UINT32(AR2316_SPI_DATA, data);			
			
			opcode = (PAGE_PROGRAM_OPCODE) | ((cyg_uint32)offset << 8);
			HAL_WRITE_UINT32(AR2316_SPI_OPCODE, opcode);
			if (page_programming_supported)
			*(volatile unsigned*)0xB1000090 &= ~(1<<page_gpio);//0xfffffffe;/*set GPIO0 to 0 to dominate spi flash CS to low active*/
    		reg = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | (first_spi_write_data_length + 4) | SPI_CTL_START;
    		HAL_WRITE_UINT32(AR2316_SPI_CTL,reg);
												
			for (i=0; i<second_direct_byte_write_data_length; i++) {
				spiflash_send_one_data_byte(cb[4+i]);
			}
		
			/*wait for spi data write complete*/
	        do {
    	        HAL_READ_UINT32(AR2316_SPI_CTL, reg);
        	} while (reg & SPI_CTL_BUSY);

			if (page_programming_supported)
			*(volatile unsigned*)0xB1000090 |= 1<<page_gpio;/*set GPIO0 to 1 to spi flash CS normal state, this will start programming*/

			//As soon as Chip Select (S) is driven High, the self-timed Page Program cycle (whose
			//duration is tPP) is initiated.
		
			//__asm("ei");//enable interrupt
				
	        while (true) {
    	        res = sfi_Command(STM_OP_RD_STATUS, 1, 1);
        	    if ((res & STM_STATUS_WIP) == 0) {
            	    break;
            	}
	        }
		}
        	
        offset += data_len;
        cb += data_len;
        len -= data_len;
    }
    return FLASH_ERR_OK;
}

#else 
int
flash_program_buf(volatile flash_t *addr, flash_t *buf, int len,
                  unsigned long block_mask, int buffer_size)
{
    cyg_uint32 res;
    cyg_uint32 offset = (cyg_uint32)addr -  CYGNUM_FLASH_BASE;
    cyg_uint32 data = 0;
    int data_len;
    cyg_uint8 *cb = (cyg_uint8 *)buf;

    while (len > 0) {
        sfi_Command(STM_OP_WR_ENABLE, 1, 0);
        data_len = len & 0x03;
        if (data_len == 0) data_len = 4;
        switch (data_len) {
        case 1:
            data = *cb;
            break;
        case 2:
            data = (cb[1] << 8) | cb[0];
            break;
        case 3:
            data = (cb[2] << 16) | (cb[1] << 8) | cb[0];
            break;
        case 4:
            data = (cb[3] << 24) | (cb[2] << 16) | (cb[1] << 8) | cb[0];
            break;
        }
        HAL_WRITE_UINT32(AR2316_SPI_DATA, data);
        sfi_Command(STM_OP_PAGE_PGRM | (offset << 8), data_len+4, 0);
        while (true) {
            res = sfi_Command(STM_OP_RD_STATUS, 1, 1);
            if ((res & STM_STATUS_WIP) == 0) {
                break;
            }
        }
        offset += data_len;
        cb += data_len;
        len -= data_len;
    }
    return FLASH_ERR_OK;
}
#endif

// EOF spiflash.c
