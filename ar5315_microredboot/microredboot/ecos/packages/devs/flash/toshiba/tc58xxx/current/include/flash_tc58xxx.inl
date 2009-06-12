#ifndef CYGONCE_DEVS_FLASH_TOSHIBA_TC58XXX_INL
#define CYGONCE_DEVS_FLASH_TOSHIBA_TC58XXX_INL
//==========================================================================
//
//      flash_tc58xxx.inl
//
//      Toshiba Tc58xxx series flash driver
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Jonathan Larmour
// Copyright (C) 2003, 2004 Gary Thomas
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
// Author(s):    Gary Thomas <gary@mlbassoc.com>
// Contributors: 
// Date:         2003-09-02
// Purpose:
// Description:  FLASH drivers for Toshiba NAND FLASH TC58xxx devices.
//               Based on Atmel AT49xxxx drivers by Jani Monoses <jani@iv.ro>
//
//####DESCRIPTIONEND####
//
//==========================================================================

// FIXME!  Someday add support for ECC data & fixups of bad sectors

#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/infra/diag.h>
#include CYGHWR_MEMORY_LAYOUT_H

#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>

// Low level debugging
//   1 - command level - prints messages about read/write/erase commands
//   2 - hardware level - shows all NAND device I/O data
#define FLASH_DEBUG  0

//----------------------------------------------------------------------------
// Common device details.
#define FLASH_Read_ID                   FLASHWORD(0x90)
#define FLASH_Reset                     FLASHWORD(0xFF)
#define FLASH_Read_Mode1                FLASHWORD(0x00)
#define FLASH_Read_Mode2                FLASHWORD(0x01)
#define FLASH_Read_Mode3                FLASHWORD(0x50)
#define FLASH_Program                   FLASHWORD(0x10)
#define FLASH_Send_Data                 FLASHWORD(0x80)
#define FLASH_Status                    FLASHWORD(0x70)
#define FLASH_Block_Erase               FLASHWORD(0x60)
#define FLASH_Start_Erase               FLASHWORD(0xD0)

#define CYGNUM_FLASH_ID_MANUFACTURER    FLASHWORD(0x98)

//----------------------------------------------------------------------------
// Now that device properties are defined, include magic for defining
// accessor type and constants.
#include <cyg/io/flash_dev.h>

//----------------------------------------------------------------------------
// Information about supported devices
typedef struct flash_dev_info {
    flash_data_t device_id;
    cyg_uint32   block_size;
    cyg_uint32   page_size;
    cyg_int32    block_count;
    cyg_uint32   base_mask;
    cyg_uint32   device_size;
} flash_dev_info_t;

static const flash_dev_info_t* flash_dev_info;
static const flash_dev_info_t supported_devices[] = {
#include <cyg/io/flash_tc58xxx_parts.inl>
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
// Initialize driver details
int
flash_hwr_init(void)
{
    flash_data_t id[4];
    int i;

#ifdef CYGHWR_FLASH_TC58XXX_PLF_INIT
    CYGHWR_FLASH_TC58XXX_PLF_INIT();
#endif

    flash_dev_query(id);

    // Check that flash_id data is matching the one the driver was
    // configured for.

    // Check manufacturer
    if (id[0] != CYGNUM_FLASH_ID_MANUFACTURER) {
        diag_printf("Can't identify FLASH - manufacturer is: %x, should be %x\n", 
                    id[0], CYGNUM_FLASH_ID_MANUFACTURER);
        return FLASH_ERR_DRV_WRONG_PART;
    }

    // Look through table for device data
    flash_dev_info = supported_devices;
    for (i = 0; i < NUM_DEVICES; i++) {
        if (flash_dev_info->device_id == id[1])
            break;
        flash_dev_info++;
    }

    // Did we find the device? If not, return error.
    if (NUM_DEVICES == i) {
        diag_printf("Can't identify FLASH - device is: %x, supported: ", id[1]);
        for (i = 0;  i < NUM_DEVICES;  i++) {
            diag_printf("%x ", supported_devices[i].device_id);
        }
        diag_printf("\n");
        return FLASH_ERR_DRV_WRONG_PART;
    }

    // Fill in device details
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

static void
put_NAND(volatile flash_data_t *ROM, flash_data_t val)
{
    *ROM = val;
#if FLASH_DEBUG > 1
    diag_printf("%02x ", val);
#endif
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
    flash_data_t* id = (flash_data_t*) data;
    volatile flash_data_t *ROM;

    ROM = (volatile flash_data_t*) CYGNUM_FLASH_BASE;
    // Send initial RESET command
    CYGHWR_FLASH_TC58XXX_CE(1);
    CYGHWR_FLASH_TC58XXX_CLE(1);
    put_NAND(ROM, FLASH_Reset);
    CYGHWR_FLASH_TC58XXX_CLE(0);
    CYGHWR_FLASH_TC58XXX_CE(0);
    // Now, wait for a good while
    CYGACC_CALL_IF_DELAY_US(10000);  // Actually 10ms
    // Issue device query
    CYGHWR_FLASH_TC58XXX_CE(1);
    CYGHWR_FLASH_TC58XXX_CLE(1);
    put_NAND(ROM, FLASH_Read_ID);
    CYGHWR_FLASH_TC58XXX_CLE(0);
    CYGHWR_FLASH_TC58XXX_ALE(1);
    put_NAND(ROM, 0x00);  // Dummy address
    CYGHWR_FLASH_TC58XXX_ALE(0);
    // Minimum 100ns delay after deasserting ALE
    CYGACC_CALL_IF_DELAY_US(10);  // Actually 10us
    id[0] = *ROM;
    id[1] = *ROM;
    CYGHWR_FLASH_TC58XXX_CLE(1);
    put_NAND(ROM, FLASH_Reset);
    CYGHWR_FLASH_TC58XXX_CLE(0);
    CYGHWR_FLASH_TC58XXX_CE(0);
}

//----------------------------------------------------------------------------
// Erase Block
int
flash_erase_block(void* block, unsigned int size)
{
    volatile flash_data_t* ROM;
    volatile flash_data_t* b_p = (volatile flash_data_t*) block;
    int res = FLASH_ERR_OK;
    int cnt = 0;
    flash_data_t stat;

#if FLASH_DEBUG > 0
    diag_printf("%s - block: %x, size: %d\n", __FUNCTION__, block, size);
#endif
    ROM = (volatile flash_data_t*) CYGNUM_FLASH_BASE;
    // Erase the next block
#if FLASH_DEBUG > 1
    diag_printf("<< ");
#endif
    CYGHWR_FLASH_TC58XXX_CE(1);
    CYGHWR_FLASH_TC58XXX_CLE(1);
    put_NAND(ROM, FLASH_Block_Erase);
    CYGHWR_FLASH_TC58XXX_CLE(0);
    CYGHWR_FLASH_TC58XXX_ALE(1);
    put_NAND(ROM, ((unsigned long)b_p & 0x0001FE00) >> 9);      // A9..A16
    put_NAND(ROM, ((unsigned long)b_p & 0x01FE0000) >> 17);     // A17..A24
    if (flash_dev_info->device_size > 0x02000000) {
        put_NAND(ROM, ((unsigned long)b_p & 0x06000000) >> 25); // A26..A27
    }
    CYGHWR_FLASH_TC58XXX_ALE(0);
    CYGHWR_FLASH_TC58XXX_CLE(1);
    put_NAND(ROM, FLASH_Start_Erase);
    CYGHWR_FLASH_TC58XXX_CLE(0);        
    CYGACC_CALL_IF_DELAY_US(10);
    while (!CYGHWR_FLASH_TC58XXX_RDY()) cnt++;  // Wait for operation to complete
    CYGHWR_FLASH_TC58XXX_CLE(1);
    put_NAND(ROM, FLASH_Status);
    CYGHWR_FLASH_TC58XXX_CLE(0);        
    stat = *ROM;
    CYGHWR_FLASH_TC58XXX_CE(0);
#if FLASH_DEBUG > 1
    diag_printf(">>\n");
#endif
#if FLASH_DEBUG > 0
    diag_printf("block: %x, stat: %x, count = %d\n", b_p, stat, cnt);
#endif
    if (stat != 0xC0) {
        diag_printf("Status after erase: %x\n", stat);
        if ((stat & 0x80) == 0x00) {
            res = FLASH_ERR_PROTECT;
        } else {
            res = FLASH_ERR_ERASE;
        }
    }
    return res;
}

//
// ECC support - adapted from Linux:
//
//  drivers/mtd/nand_ecc.c
//
//  Copyright (C) 2000 Steven J. Hill (sjhill@realitydiluted.com)
//                     Toshiba America Electronics Components, Inc.
//

static const unsigned char _nand_ecc_precalc_table[] = {
    0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a, 0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00,
    0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f, 0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
    0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c, 0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
    0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59, 0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
    0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33, 0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
    0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56, 0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
    0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55, 0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
    0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30, 0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
    0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30, 0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
    0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55, 0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
    0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56, 0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
    0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33, 0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
    0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59, 0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
    0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c, 0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
    0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f, 0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
    0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a, 0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00
};

static void 
_nand_trans_result(unsigned char reg2, unsigned char reg3,
                   unsigned char *ecc0, unsigned char *ecc1)
{
    unsigned char a, b, i, tmp1, tmp2;
	
    /* Initialize variables */
    a = b = 0x80;
    tmp1 = tmp2 = 0;
	
    /* Calculate first ECC byte */
    for (i = 0; i < 4; i++) {
        if (reg3 & a)		/* LP15,13,11,9 --> ecc_code[0] */
            tmp1 |= b;
        b >>= 1;
        if (reg2 & a)		/* LP14,12,10,8 --> ecc_code[0] */
            tmp1 |= b;
        b >>= 1;
        a >>= 1;
    }
	
    /* Calculate second ECC byte */
    b = 0x80;
    for (i = 0; i < 4; i++) {
        if (reg3 & a)		/* LP7,5,3,1 --> ecc_code[1] */
            tmp2 |= b;
        b >>= 1;
        if (reg2 & a)		/* LP6,4,2,0 --> ecc_code[1] */
            tmp2 |= b;
        b >>= 1;
        a >>= 1;
    }
	
    /* Store two of the ECC bytes */
    *ecc0 = tmp1;    
    *ecc1 = tmp2;
}

//
// Calculate 3 byte ECC on 256 bytes of data
//
static void
_nand_page_ECC(unsigned char *data, unsigned char *ecc0,
               unsigned char *ecc1, unsigned char *ecc2)
{
    unsigned char idx, reg1, reg2, reg3;
    int j;
	
    /* Initialize variables */
    reg1 = reg2 = reg3 = 0;
    *ecc0 = *ecc1 = *ecc2 = 0;
	
    /* Build up column parity */ 
    for(j = 0; j < 256; j++) {
        /* Get CP0 - CP5 from table */
        idx = _nand_ecc_precalc_table[*data++];
        reg1 ^= (idx & 0x3f);
        /* All bit XOR = 1 ? */
        if (idx & 0x40) {
            reg3 ^= (unsigned char) j;
            reg2 ^= ~((unsigned char) j);
        }
    }
	
    /* Create non-inverted ECC code from line parity */
    _nand_trans_result(reg2, reg3, ecc0, ecc1);
	
    /* Calculate final ECC code */
    *ecc0 = ~*ecc0;
    *ecc1 = ~*ecc1;
    *ecc2 = ((~reg1) << 2) | 0x03;
}

//
// Correct a buffer via ECC (1 bit, 256 byte block)
//  Return: 0 => No error
//          1 => Corrected
//          2 => Not corrected, ECC updated
//         -1 => Not correctable
//
int 
_nand_correct_data(unsigned char *dat, unsigned char *read_ecc, unsigned char *calc_ecc)
{
    unsigned char a, b, c, d1, d2, d3, add, bit, i;
	
    /* Do error detection */ 
    d1 = calc_ecc[0] ^ read_ecc[0];
    d2 = calc_ecc[1] ^ read_ecc[1];
    d3 = calc_ecc[2] ^ read_ecc[2];
	
    if ((d1 | d2 | d3) == 0) {
        /* No errors */
        return 0;
    } else {
        a = (d1 ^ (d1 >> 1)) & 0x55;
        b = (d2 ^ (d2 >> 1)) & 0x55;
        c = (d3 ^ (d3 >> 1)) & 0x54;
		
        /* Found and will correct single bit error in the data */
        if ((a == 0x55) && (b == 0x55) && (c == 0x54)) {
            c = 0x80;
            add = 0;
            a = 0x80;
            for (i=0; i<4; i++) {
                if (d1 & c)
                    add |= a;
                c >>= 2;
                a >>= 1;
            }
            c = 0x80;
            for (i=0; i<4; i++) {
                if (d2 & c)
                    add |= a;
                c >>= 2;
                a >>= 1;
            }
            bit = 0;
            b = 0x04;
            c = 0x80;
            for (i=0; i<3; i++) {
                if (d3 & c)
                    bit |= b;
                c >>= 2;
                b >>= 1;
            }
            b = 0x01;
            a = dat[add];
            a ^= (b << bit);
            dat[add] = a;
            return 1;
        } else {
            i = 0;
            while (d1) {
                if (d1 & 0x01)
                    ++i;
                d1 >>= 1;
            }
            while (d2) {
                if (d2 & 0x01)
                    ++i;
                d2 >>= 1;
            }
            while (d3) {
                if (d3 & 0x01)
                    ++i;
                d3 >>= 1;
            }
            if (i == 1) {
                /* ECC Code Error Correction */
                read_ecc[0] = calc_ecc[0];
                read_ecc[1] = calc_ecc[1];
                read_ecc[2] = calc_ecc[2];
                return 2;
            } else {
                /* Uncorrectable Error */
                return -1;
            }
        }
    }
	
    /* Should never happen */
    return -1;
}


//----------------------------------------------------------------------------
// Program Buffer
int
flash_program_buf(void* addr, void* data, int len)
{
    volatile flash_data_t* ROM;
    volatile flash_data_t* addr_ptr = (volatile flash_data_t*) addr;
    volatile flash_data_t* data_ptr = (volatile flash_data_t*) data;
    int res = FLASH_ERR_OK;
    int i, cnt;
    flash_data_t stat;
    unsigned char oob[16];

    ROM = (volatile flash_data_t*) CYGNUM_FLASH_BASE;
#if FLASH_DEBUG > 0
    diag_printf("%s - addr: %x, data: %x, len: %d, FLASH: %p/%d\n", 
                __FUNCTION__, addr, data, len, ROM, sizeof(flash_data_t));
#endif
    while (len > 0) {
        CYGHWR_FLASH_TC58XXX_CE(1);
        CYGHWR_FLASH_TC58XXX_CLE(1);
#if FLASH_DEBUG > 1
        diag_printf("<< ");
#endif
        put_NAND(ROM, FLASH_Read_Mode1);
        put_NAND(ROM, FLASH_Send_Data);
        CYGHWR_FLASH_TC58XXX_CLE(0);
        CYGHWR_FLASH_TC58XXX_ALE(1);
        put_NAND(ROM, ((unsigned long)addr_ptr & 0x000000FF) >> 0);   // A0..A7
        put_NAND(ROM, ((unsigned long)addr_ptr & 0x0001FE00) >> 9);   // A9..A16
        put_NAND(ROM, ((unsigned long)addr_ptr & 0x01FE0000) >> 17);  // A17..A24
        if (flash_dev_info->device_size > 0x02000000) {
            put_NAND(ROM, ((unsigned long)addr_ptr & 0x06000000) >> 25);  // A26..A27
        }
        CYGHWR_FLASH_TC58XXX_ALE(0);
#if FLASH_DEBUG > 1
        diag_printf(">>\n");
#endif
        // Caculate OOB data for page (ECC)
        for (i = 0;  i < 16;  i++) {
            oob[i] = 0xFF;
        }
        // Calculate ECC for page
        _nand_page_ECC((unsigned char *)&data_ptr[0], &oob[0], &oob[1], &oob[2]);
        _nand_page_ECC((unsigned char *)&data_ptr[256], &oob[3], &oob[6], &oob[7]);
        // Move one page of data to buffer
        for (i = 0;  i < 512;  i++) {
            put_NAND(ROM, *data_ptr++);
#if FLASH_DEBUG > 1
            if ((i % 16) == 15) diag_printf("\n");
#endif
        }
        // OOB data
        for (i = 0;  i < 16;  i++) {
            put_NAND(ROM, oob[i]);
#if FLASH_DEBUG > 1
            if ((i % 16) == 15) diag_printf("\n");
#endif
        }
#if FLASH_DEBUG > 1
        diag_printf("<< ");
#endif
        CYGHWR_FLASH_TC58XXX_CLE(1);
        put_NAND(ROM, FLASH_Program);
        CYGHWR_FLASH_TC58XXX_CLE(0);
        CYGACC_CALL_IF_DELAY_US(1);  // Actually 200ns
        cnt = 0;
        CYGACC_CALL_IF_DELAY_US(10);
        while (!CYGHWR_FLASH_TC58XXX_RDY()) cnt++;  // Wait for page data to be ready
        CYGHWR_FLASH_TC58XXX_CLE(1);
        put_NAND(ROM, FLASH_Status);
        CYGHWR_FLASH_TC58XXX_CLE(0);        
#if FLASH_DEBUG > 1
        diag_printf(">>\n");
#endif
        stat = *ROM;
        CYGHWR_FLASH_TC58XXX_CE(0);        
#if FLASH_DEBUG > 0
        diag_printf("program at %x, stat: %x, count = %d\n", addr_ptr, stat, cnt);
#endif
        addr_ptr += 512;  len -= 512;
        if (stat != 0xC0) {
            diag_printf("Status after write: %x\n", stat);
            if ((stat & 0x80) == 0x00) {
                res = FLASH_ERR_PROTECT;
            } else {
                res = FLASH_ERR_PROGRAM;
            }
        }
    }
    return res;
}

//----------------------------------------------------------------------------
// Read data into buffer
int
flash_read_buf(void* addr, void* data, int len)
{
    volatile flash_data_t* ROM;
    volatile flash_data_t* addr_ptr = (volatile flash_data_t*) addr;
    volatile flash_data_t* data_ptr = (volatile flash_data_t*) data;
    flash_data_t *page;
    int res = FLASH_ERR_OK;
    int i, cnt, offset;
    flash_data_t stat;
    unsigned char oob[16], dev_oob[16];

    ROM = (volatile flash_data_t*) CYGNUM_FLASH_BASE;
#if FLASH_DEBUG > 1
    diag_printf("<< ");
#endif
    CYGHWR_FLASH_TC58XXX_CE(1);
    CYGHWR_FLASH_TC58XXX_CLE(1);
    if (((unsigned long)addr & 0x100) == 0) {
        // Mode 1 - reads from start of line
        put_NAND(ROM, FLASH_Read_Mode1);
    } else {
        // Mode 2 - reads from second half of line
        put_NAND(ROM, FLASH_Read_Mode2);
    }
    CYGHWR_FLASH_TC58XXX_CLE(0);
    CYGHWR_FLASH_TC58XXX_ALE(1);
    put_NAND(ROM, ((unsigned long)addr_ptr & 0x000000FF) >> 0);   // A0..A7
    put_NAND(ROM, ((unsigned long)addr_ptr & 0x0001FE00) >> 9);   // A9..A16
    put_NAND(ROM, ((unsigned long)addr_ptr & 0x01FE0000) >> 17);  // A17..A24
    if (flash_dev_info->device_size > 0x02000000) {
        put_NAND(ROM, ((unsigned long)addr_ptr & 0x06000000) >> 25);  // A26..A27
    }
    CYGHWR_FLASH_TC58XXX_ALE(0);
#if FLASH_DEBUG > 1
    diag_printf(">>\n");
#endif
    cnt = 0;
    CYGACC_CALL_IF_DELAY_US(10);
    while (!CYGHWR_FLASH_TC58XXX_RDY()) cnt++;  // Wait for page data to be ready 
#if FLASH_DEBUG > 0
    diag_printf("Read data starting at %p, count = %d\n", data_ptr, cnt);
#endif
    offset = 0;
    page = (unsigned char *)data_ptr;
    while (len-- > 0) {
        *data_ptr++ = *ROM;
        if (++offset == 0x200) {
            // Data page has been read, fetch ECC/OOB data
            for (i = 0;  i < 16;  i++) {
                dev_oob[i] = *ROM;
            }
            // Calculate actual ECC on page
            _nand_page_ECC(&page[0], &oob[0], &oob[1], &oob[2]);
            _nand_page_ECC(&page[256], &oob[3], &oob[6], &oob[7]);
            // Check & repair if possible
            if ((oob[0] != dev_oob[0]) || (oob[1] != dev_oob[1]) || (oob[2] != dev_oob[2]) || 
                (oob[3] != dev_oob[3]) || (oob[6] != dev_oob[6]) || (oob[7] != dev_oob[7])) {
                unsigned char read_ecc[3], calc_ecc[3];
                int res;

                read_ecc[0] = dev_oob[0];  calc_ecc[0] = oob[0];
                read_ecc[1] = dev_oob[1];  calc_ecc[1] = oob[1];
                read_ecc[2] = dev_oob[2];  calc_ecc[2] = oob[2];
                res = _nand_correct_data(&page[0], read_ecc, calc_ecc);
                if ((res != 0) && (res != 1)) {
                    diag_printf("ECC failed\n");
                    res = FLASH_ERR_HWR;
                    break;
                }
                read_ecc[0] = dev_oob[3];  calc_ecc[0] = oob[3];
                read_ecc[1] = dev_oob[6];  calc_ecc[1] = oob[6];
                read_ecc[2] = dev_oob[7];  calc_ecc[2] = oob[7];
                res = _nand_correct_data(&page[256], read_ecc, calc_ecc);
                if ((res != 0) && (res != 1)) {
                    diag_printf("ECC failed\n");
                    res = FLASH_ERR_HWR;
                    break;
                }
            }
            cnt = 0;
            CYGACC_CALL_IF_DELAY_US(10);
            while (!CYGHWR_FLASH_TC58XXX_RDY()) cnt++;  // Wait for page data to be ready
#if FLASH_DEBUG > 0
            diag_printf("Read data starting at %p, count = %d\n", data_ptr, cnt);
#endif
            offset = 0;
            page = (unsigned char *)data_ptr;
        }
    }
    CYGHWR_FLASH_TC58XXX_CLE(1);
    put_NAND(ROM, FLASH_Status);
    CYGHWR_FLASH_TC58XXX_CLE(0);        
    stat = *ROM;
    CYGHWR_FLASH_TC58XXX_CE(0);
    if (stat != 0xC0) {
        diag_printf("Status after read: %x\n", stat);
        res = FLASH_ERR_HWR;
    }
    return res;
}

#endif // CYGONCE_DEVS_FLASH_TOSHIBA_TC58XXX_INL

// EOF flash_tc58xxx.inl
