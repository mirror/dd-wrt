//=============================================================================
//
//      hal_aux.c
//
//      HAL auxiliary objects and code; per platform
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   hmt
// Contributors:hmt
// Date:        1999-06-08
// Purpose:     HAL aux objects: startup tables.
// Description: Tables for per-platform initialization
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_mem.h>            // HAL memory definitions
#include <pkgconf/hal_powerpc_quicc.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/quicc/ppc8xx.h>
#include <cyg/hal/hal_if.h>             // hal_if_init

// The memory map is weakly defined, allowing the application to redefine
// it if necessary. The regions defined below are the minimum requirements.
CYGARC_MEMDESC_TABLE CYGBLD_ATTRIB_WEAK = {
    // Mapping for the Motorola MBX860 development board
    CYGARC_MEMDESC_CACHE(   0xfe000000, 0x00400000 ), // ROM region
    CYGARC_MEMDESC_NOCACHE( 0xfa000000, 0x00400000 ), // Control/Status+LEDs
    CYGARC_MEMDESC_CACHE(   0x00000000, 0x00800000 ), // Main memory

    CYGARC_MEMDESC_TABLE_END
};

#ifdef _DOWNLOAD_UCODE_UPDATE // Not currently used
//
// The MPC8xx CPM (Control Processor) has some problems (overlapping structures) which
// can be fixed by downloading new ucode.  This code came from:
//   http://www.mot.com/SPS/ADC/pps/subpgs/etoolbox/8XX/i2c_spi.html
//

static unsigned char i2c_ucode_low[] = {
    0x7F, 0xFF, 0xEF, 0xD9, 0x3F, 0xFD, 0x00, 0x00,
    0x7F, 0xFB, 0x49, 0xF7, 0x7F, 0xF9, 0x00, 0x00,
    0x5F, 0xEF, 0xAD, 0xF7, 0x5F, 0x89, 0xAD, 0xF7,
    0x5F, 0xEF, 0xAF, 0xF7, 0x5F, 0x89, 0xAF, 0xF7,
    0x3A, 0x9C, 0xFB, 0xC8, 0xE7, 0xC0, 0xED, 0xF0,
    0x77, 0xC1, 0xE1, 0xBB, 0xF4, 0xDC, 0x7F, 0x1D,
    0xAB, 0xAD, 0x93, 0x2F, 0x4E, 0x08, 0xFD, 0xCF,
    0x6E, 0x0F, 0xAF, 0xF8, 0x7C, 0xCF, 0x76, 0xCF,
    0xFD, 0x1F, 0xF9, 0xCF, 0xAB, 0xF8, 0x8D, 0xC6,
    0xAB, 0x56, 0x79, 0xF7, 0xB0, 0x93, 0x73, 0x83,
    0xDF, 0xCE, 0x79, 0xF7, 0xB0, 0x91, 0xE6, 0xBB,
    0xE5, 0xBB, 0xE7, 0x4F, 0xB3, 0xFA, 0x6F, 0x0F,
    0x6F, 0xFB, 0x76, 0xCE, 0xEE, 0x0D, 0xF9, 0xCF,
    0x2B, 0xFB, 0xEF, 0xEF, 0xCF, 0xEE, 0xF9, 0xCF,
    0x76, 0xCE, 0xAD, 0x24, 0x90, 0xB2, 0xDF, 0x9A,
    0x7F, 0xDD, 0xD0, 0xBF, 0x4B, 0xF8, 0x47, 0xFD,
    0x7C, 0xCF, 0x76, 0xCE, 0xCF, 0xEF, 0x7E, 0x1F,
    0x7F, 0x1D, 0x7D, 0xFD, 0xF0, 0xB6, 0xEF, 0x71,
    0x7F, 0xC1, 0x77, 0xC1, 0xFB, 0xC8, 0x60, 0x79,
    0xE7, 0x22, 0xFB, 0xC8, 0x5F, 0xFF, 0xDF, 0xFF,
    0x5F, 0xB2, 0xFF, 0xFB, 0xFB, 0xC8, 0xF3, 0xC8,
    0x94, 0xA6, 0x7F, 0x01, 0x7F, 0x1D, 0x5F, 0x39,
    0xAF, 0xE8, 0x5F, 0x5E, 0xFF, 0xDF, 0xDF, 0x96,
    0xCB, 0x9F, 0xAF, 0x7D, 0x5F, 0xC1, 0xAF, 0xED,
    0x8C, 0x1C, 0x5F, 0xC1, 0xAF, 0xDD, 0x5F, 0xC3,
    0xDF, 0x9A, 0x7E, 0xFD, 0xB0, 0xB2, 0x5F, 0xB2,
    0xFF, 0xFE, 0xAB, 0xAD, 0x5F, 0xB2, 0xFF, 0xFE,
    0x5F, 0xCE, 0x60, 0x0B, 0xE6, 0xBB, 0x60, 0x0B,
    0x5F, 0xCE, 0xDF, 0xC6, 0x27, 0xFB, 0xEF, 0xDF,
    0x5F, 0xC8, 0xCF, 0xDE, 0x3A, 0x9C, 0xE7, 0xC0,
    0xED, 0xF0, 0xF3, 0xC8, 0x7F, 0x01, 0x54, 0xCD,
    0x7F, 0x1D, 0x2D, 0x3D, 0x36, 0x3A, 0x75, 0x70,
    0x7E, 0x0A, 0xF1, 0xCE, 0x37, 0xEF, 0x2E, 0x68,
    0x7F, 0xEE, 0x10, 0xEC, 0xAD, 0xF8, 0xEF, 0xDE,
    0xCF, 0xEA, 0xE5, 0x2F, 0x7D, 0x0F, 0xE1, 0x2B,
    0xF1, 0xCE, 0x5F, 0x65, 0x7E, 0x0A, 0x4D, 0xF8,
    0xCF, 0xEA, 0x5F, 0x72, 0x7D, 0x0B, 0xEF, 0xEE,
    0xCF, 0xEA, 0x5F, 0x74, 0xE5, 0x22, 0xEF, 0xDE,
    0x5F, 0x74, 0xCF, 0xDA, 0x0B, 0x62, 0x73, 0x85,
    0xDF, 0x62, 0x7E, 0x0A, 0x30, 0xD8, 0x14, 0x5B,
    0xBF, 0xFF, 0xF3, 0xC8, 0x5F, 0xFF, 0xDF, 0xFF,
    0xA7, 0xF8, 0x5F, 0x5E, 0xBF, 0xFE, 0x7F, 0x7D,
    0x10, 0xD3, 0x14, 0x50, 0x5F, 0x36, 0xBF, 0xFF,
    0xAF, 0x78, 0x5F, 0x5E, 0xBF, 0xFD, 0xA7, 0xF8,
    0x5F, 0x36, 0xBF, 0xFE, 0x77, 0xFD, 0x30, 0xC0,
    0x4E, 0x08, 0xFD, 0xCF, 0xE5, 0xFF, 0x6E, 0x0F,
    0xAF, 0xF8, 0x7E, 0x1F, 0x7E, 0x0F, 0xFD, 0x1F,
    0xF1, 0xCF, 0x5F, 0x1B, 0xAB, 0xF8, 0x0D, 0x5E,
    0x5F, 0x5E, 0xFF, 0xEF, 0x79, 0xF7, 0x30, 0xA2,
    0xAF, 0xDD, 0x5F, 0x34, 0x47, 0xF8, 0x5F, 0x34,
    0xAF, 0xED, 0x7F, 0xDD, 0x50, 0xB2, 0x49, 0x78,
    0x47, 0xFD, 0x7F, 0x1D, 0x7D, 0xFD, 0x70, 0xAD,
    0xEF, 0x71, 0x7E, 0xC1, 0x6B, 0xA4, 0x7F, 0x01,
    0x2D, 0x26, 0x7E, 0xFD, 0x30, 0xDE, 0x5F, 0x5E,
    0xFF, 0xFD, 0x5F, 0x5E, 0xFF, 0xEF, 0x5F, 0x5E,
    0xFF, 0xDF, 0x0C, 0xA0, 0xAF, 0xED, 0x0A, 0x9E,
    0xAF, 0xDD, 0x0C, 0x3A, 0x5F, 0x3A, 0xAF, 0xBD,
    0x7F, 0xBD, 0xB0, 0x82, 0x5F, 0x82, 0x47, 0xF8,
};
static unsigned char i2c_ucode_high[] = {
    0x3E, 0x30, 0x34, 0x30, 0x34, 0x34, 0x37, 0x37,
    0xAB, 0xF7, 0xBF, 0x9B, 0x99, 0x4B, 0x4F, 0xBD,
    0xBD, 0x59, 0x94, 0x93, 0x34, 0x9F, 0xFF, 0x37,
    0xFB, 0x9B, 0x17, 0x7D, 0xD9, 0x93, 0x69, 0x56,
    0xBB, 0xFD, 0xD6, 0x97, 0xBD, 0xD2, 0xFD, 0x11,
    0x31, 0xDB, 0x9B, 0xB3, 0x63, 0x13, 0x96, 0x37,
    0x93, 0x73, 0x36, 0x93, 0x19, 0x31, 0x37, 0xF7,
    0x33, 0x17, 0x37, 0xAF, 0x7B, 0xB9, 0xB9, 0x99,
    0xBB, 0x19, 0x79, 0x57, 0x7F, 0xDF, 0xD3, 0xD5,
    0x73, 0xB7, 0x73, 0xF7, 0x37, 0x93, 0x3B, 0x99,
    0x1D, 0x11, 0x53, 0x16, 0x99, 0x31, 0x53, 0x15,
    0x31, 0x69, 0x4B, 0xF4, 0xFB, 0xDB, 0xD3, 0x59,
    0x31, 0x49, 0x73, 0x53, 0x76, 0x95, 0x6D, 0x69,
    0x7B, 0x9D, 0x96, 0x93, 0x13, 0x13, 0x19, 0x79,
    0x79, 0x37, 0x69, 0x35, 
};
#endif

#define QUICC_I2C_MOD_EN          0x01  // Enable I2C controller
#define QUICC_I2C_MOD_PDIV_CLK32  0x00  // I2C BRG - /32
#define QUICC_I2C_MOD_PDIV_CLK16  0x02  //           /16
#define QUICC_I2C_MOD_PDIV_CLK8   0x04  //           /8
#define QUICC_I2C_MOD_PDIV_CLK4   0x06  //           /4
#define QUICC_I2C_MOD_FLT         0x08  // 1 = input clock is filtered
#define QUICC_I2C_MOD_GCD         0x10  // 1 = deny general call address
#define QUICC_I2C_MOD_REVD        0x20  // 1 = reverse TxD and RxD order

#define QUICC_I2C_CMD_MASTER      0x01  // 1 = master, 0 = slave
#define QUICC_I2C_CMD_START       0x80  // 1 = start transmit

#define QUICC_I2C_FCR_BE          0x10  // Big Endian operation

#define MBX_CONFIG_EEPROM         0xA5  // I2C address of configuration ROM

static unsigned char _MBX_eeprom_data[0x100];
#define VPD_EOD 0xFF

static void
_mbx_init_i2c(void)
{
    volatile EPPC *eppc = eppc_base();
    unsigned char *sp, *ep;
    int i, len, RxBD, TxBD;
    struct i2c_pram *i2c;
    volatile struct cp_bufdesc *rxbd, *txbd;    
    unsigned char i2c_address[521];
    static int i2c_init = 0;

    if (i2c_init) return;
    i2c_init = 1;
    eppc->cp_rccr = 0;  // Disables any current ucode running
#ifdef _DOWNLOAD_UCODE_UPDATE // Not currently used
    // Patch the ucode
    sp = i2c_ucode_low;
    ep = (unsigned char *)eppc->udata_ucode;
    for (i = 0;  i < sizeof(i2c_ucode_low);  i++) {
        *ep++ = *sp++;
    }
    sp = i2c_ucode_high;
    ep = (unsigned char *)eppc->udata_ext;
    for (i = 0;  i < sizeof(i2c_ucode_high);  i++) {
        *ep++ = *sp++;
    }
    eppc->cp_rctr1 = 0x802A;
    eppc->cp_rctr2 = 0x8028;
    eppc->cp_rctr3 = 0x802E;
    eppc->cp_rctr4 = 0x802C;
    diag_printf("RCCR: %x, RTCRx: %x/%x/%x/%x\n",
                eppc->cp_rccr, eppc->cp_rctr1, eppc->cp_rctr2, eppc->cp_rctr3, eppc->cp_rctr4);
    diag_dump_buf(eppc->udata_ucode, 256);
    diag_dump_buf(&eppc->pram[0].scc.pothers.i2c_idma, 0x40);
    eppc->cp_rccr = 0x01;  // Enable ucode
    diag_printf("RCCR: %x, RTCRx: %x/%x/%x/%x\n",
                eppc->cp_rccr, eppc->cp_rctr1, eppc->cp_rctr2, eppc->cp_rctr3, eppc->cp_rctr4);
    diag_dump_buf(eppc->udata_ucode, 256);
    diag_dump_buf(&eppc->pram[0].scc.pothers.i2c_idma, 0x40);
    diag_printf("RPBASE = %x/%x\n", eppc->pram[0].scc.pothers.i2c_idma.i2c.rpbase, &eppc->pram[0].scc.pothers.i2c_idma.i2c.rpbase);
    eppc->pram[0].scc.pothers.i2c_idma.i2c.rpbase = (unsigned long)&eppc->i2c_spare_pram - (unsigned long)eppc;
    diag_printf("RPBASE = %x/%x\n", eppc->pram[0].scc.pothers.i2c_idma.i2c.rpbase, &eppc->pram[0].scc.pothers.i2c_idma.i2c.rpbase);
    eppc->i2c_i2mod = 0;  // Disable I2C controller
    i2c = (struct i2c_pram *)&eppc->i2c_spare_pram;
#else
    eppc->i2c_i2mod = 0;  // Disable I2C controller
    i2c = (struct i2c_pram *)&eppc->pram[0].scc.pothers.i2c_idma.i2c;
#endif // _DOWNLOAD_UCODE_UPDATE
    sp = (unsigned char *)i2c;
    for (i = 0;  i < sizeof(*i2c);  i++) {
        *sp++ = 0;
    }
    RxBD = 0x2E08;  // CAUTION
    TxBD = 0x2E00;
    i2c->rbase = RxBD;
    i2c->tbase = TxBD;
    i2c->rfcr = QUICC_I2C_FCR_BE;
    i2c->tfcr = QUICC_I2C_FCR_BE;
    i2c->mrblr = sizeof(i2c_address);
    rxbd = (volatile struct cp_bufdesc *)((char *)eppc + RxBD);
    rxbd->ctrl = QUICC_BD_CTL_Ready | QUICC_BD_CTL_Wrap | QUICC_BD_CTL_Last;
    rxbd->length = 257;
    rxbd->buffer = i2c_address;
    txbd = (volatile struct cp_bufdesc *)((char *)eppc + TxBD);
    txbd->length = 1+520;
    i2c_address[0] = MBX_CONFIG_EEPROM;
    txbd->buffer = i2c_address;
    txbd->ctrl = QUICC_BD_CTL_Ready | QUICC_BD_CTL_Wrap | QUICC_BD_CTL_Last;
    eppc->i2c_i2add = 0x00;
    eppc->i2c_i2brg = 0x50;
    eppc->i2c_i2mod = QUICC_I2C_MOD_EN;  // Enable I2C interface
    // Initialize the CPM (set up buffer pointers, etc).
    // This needs to be done *after* the interface is enabled.
    eppc->cp_cr = QUICC_CPM_I2C | QUICC_CPM_CR_INIT_RX | QUICC_CPM_CR_BUSY;
    while (eppc->cp_cr & QUICC_CPM_CR_BUSY) ;
    eppc->cp_cr = QUICC_CPM_I2C | QUICC_CPM_CR_INIT_TX | QUICC_CPM_CR_BUSY;
    while (eppc->cp_cr & QUICC_CPM_CR_BUSY) ;
    eppc->i2c_i2com = QUICC_I2C_CMD_MASTER | QUICC_I2C_CMD_START;
    i = 0;
    while (txbd->ctrl & QUICC_BD_CTL_Ready) {
        if (++i > 50000) break;
    }
    // Rebuild the actual VPD
    for (i = 0;  i < sizeof(_MBX_eeprom_data);  i++) {
        _MBX_eeprom_data[i] = VPD_EOD;  // Undefined
    }
    sp = (unsigned char *)&i2c_address[1];
    ep = (unsigned char *)&i2c_address[sizeof(i2c_address)];
    while (sp != ep) {
        if ((sp[0] == 'M') && (sp[1] == 'O') && (sp[2] == 'T')) {
            // Found the "eye catcher" string
            sp += 8;
            len = (sp[0] << 8) | sp[1];
            sp += 2;
            for (i = 0;  i < len;  i++) {
                _MBX_eeprom_data[i] = *sp++;
                if (sp == ep) sp = (unsigned char *)&i2c_address[1];
            }
            break;
        }
        sp++;
    }
    eppc->i2c_i2mod = 0;  // Disable I2C interface
}

//
// Fetch a value from the VPD and return it's length.
// Returns a length of zero if not found
//
int
_mbx_fetch_VPD(int code, unsigned char *buf, int size)
{
    unsigned char *vp, *ep;
    int i, len;

    _mbx_init_i2c();  // Fetch the data if not already
    vp = &_MBX_eeprom_data[0];
    ep = &_MBX_eeprom_data[sizeof(_MBX_eeprom_data)];
    while (vp < ep) {
        if (*vp == (unsigned char)code) {
            // Found the desired item
            len = (int)vp[1];
            if (len > size) len = size;
            vp += 2;            
            for (i = 0;  i < len;  i++) {
                *buf++ = *vp++;
            }
            return len;
        }
        len = (int)vp[1];
        vp += 2 + len;  // Skip to next item
        if (*vp == VPD_EOD) break;
    }
    return 0;
}

//--------------------------------------------------------------------------
// Platform init code.
void
hal_platform_init(void)
{
    hal_if_init();
}

// EOF hal_aux.c
