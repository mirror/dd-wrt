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
// Copyright (C) 2002, 2003 Gary Thomas
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
// Contributors:hmt, gthomas
// Date:        1999-06-08
// Purpose:     HAL aux objects: startup tables.
// Description: Tables for per-platform initialization
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_mem.h>            // HAL memory definitions
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/mpc8xxx.h>            // For IMM structures
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_intr.h>

// FIXME

static __inline__ unsigned long
_le32(unsigned long val)
{
    return (((val & 0x000000FF) << 24) |
            ((val & 0x0000FF00) <<  8) |
            ((val & 0x00FF0000) >>  8) |
            ((val & 0xFF000000) >> 24));
}

static __inline__ unsigned short
_le16(unsigned short val)
{
    return (((val & 0x000000FF) << 8) |
            ((val & 0x0000FF00) >> 8));
}

#define HAL_WRITE_UINT32LE(_addr_, _val_) \
  HAL_WRITE_UINT32(_addr_, _le32(_val_))
#define HAL_WRITE_UINT16LE(_addr_, _val_) \
  HAL_WRITE_UINT16(_addr_, _le16(_val_))
#define HAL_WRITE_UINT8LE(_addr_, _val_) \
  HAL_WRITE_UINT8(_addr_, _val_)
#define HAL_READ_UINT32LE(_addr_, _val_)        \
  {                                             \
      HAL_READ_UINT32(_addr_, _val_);           \
      _val_ = _le32(_val_);                     \
  }
#define HAL_READ_UINT16LE(_addr_, _val_)        \
  {                                             \
      HAL_READ_UINT16(_addr_, _val_);           \
      _val_ = _le16(_val_);                     \
  }
#define HAL_READ_UINT8LE(_addr_, _val_)        \
  HAL_READ_UINT8(_addr_, _val_)

// FIXME

// The memory map is weakly defined, allowing the application to redefine
// it if necessary. The regions defined below are the minimum requirements.
CYGARC_MEMDESC_TABLE CYGBLD_ATTRIB_WEAK = {
    // Mapping for the Rattler board
    CYGARC_MEMDESC_CACHE( 0x00000000, 0x01000000 ), // Main memory 60x SDRAM
    CYGARC_MEMDESC_CACHE( 0xFE000000, 0x00800000 ), // ROM region
    CYGARC_MEMDESC_NOCACHE( 0x80000000, 0x00100000 ), // Extended I/O
    CYGARC_MEMDESC_NOCACHE( 0xFF000000, 0x00100000 ), // IMMR registers

    CYGARC_MEMDESC_TABLE_END
};

//--------------------------------------------------------------------------
// Platform init code.
void
hal_platform_init(void)
{
#ifndef CYGSEM_HAL_USE_ROM_MONITOR
    volatile t_PQ2IMM  *IMM = (volatile t_PQ2IMM *)CYGARC_IMM_BASE;
    cyg_bool old_board_layout = *(unsigned long *)0xFE0000FC == 0;

    // Configure the I/O pins used by this board
    //   + = PARx = 0, PSORx = 0, PDIRx = 0
    //   A = PARX = 1, PSORx = 0, PDIRx = 0
    //   B = PARX = 1, PSORx = 1, PDIRx = 0
    //   C = PARX = 1, PSORx = 0, PDIRx = 1
    //   D = PARX = 1, PSORx = 1, PDIRx = 1

    // Port A
    //             1111111111222222222233
    //   01234567890123456789012345678901
    //   _______________________________________
    //                                  B  FCC1 - COL/MII
    //                                 B-  FCC1 - SRC/MII
    //                                D--  FCC1 - TX_ER/MII
    //                               D---  FCC1 - TX_EN/MII
    //                              B----  FCC1 - RX_DV/MII
    //                             B-----  FCC1 - RX_ER/MII
    //                            +------
    //                           +-------
    //                          +--------
    //                         +---------
    //                        C----------  FCC1 - TX[3]/MII
    //                       C-----------  FCC1 - TX[2]/MII
    //                      C------------  FCC1 - TX{1]/MII
    //                     C-------------  FCC1 - TX[0]/MII
    //                    A--------------  FCC1 - RX[3]/MII
    //                   A---------------  FCC1 - RX[2]/MII
    //                  A----------------  FCC1 - RX[1]/MII
    //                 A-----------------  FCC1 - RX[0]/MII
    //                +------------------
    //               +-------------------
    //              +--------------------
    //             +---------------------
    //            +----------------------
    //           +-----------------------
    //          +------------------------
    //         +-------------------------
    //        +--------------------------
    //       +---------------------------
    //      +----------------------------
    //     +-----------------------------  LED2
    //    +------------------------------  LED1
    //   +-------------------------------  LED0
    //   ++++ ++++ ++++ ++AA AACC CC++ ++BB DDBB
    //   0000 0000 0000 0011 1111 1100 0011 1111 PAR
    //   0000 0000 0000 0000 0000 0000 0011 1111 SOR
    //   0000 0000 0000 0000 0011 1100 0000 1100 DIR
    IMM->io_regs[PORT_A].ppar = 0x0003FC3F;
    IMM->io_regs[PORT_A].psor = 0x0000003F;
    IMM->io_regs[PORT_A].pdir = 0xE0003C0C;
    IMM->io_regs[PORT_A].podr = 0x00000000;

    // Port B
    //             1111111111222222222233
    //   01234567890123456789012345678901
    //   ________________________________
    //                                  C FCC2 - TX_ER
    //                                 A- FCC2 - RX_DV
    //                                D-- FCC2 - TX_EN
    //                               A--- FCC2 - RX_ER
    //                              A---- FCC2 - COL
    //                             A----- FCC2 - CRS
    //                            C------ FCC2 - TxD[3]
    //                           C------- FCC2 - TxD[2]
    //                          C-------- FCC2 - TxD[1]
    //                         C--------- FCC2 - TxD[0]
    //                        A---------- FCC2 - RxD[0]
    //                       A----------- FCC2 - RxD[1]
    //                      A------------ FCC2 - RxD[2]
    //                     A------------- FCC2 - RxD[3]
    //                    +--------------
    //                   +---------------
    //                  +----------------
    //                 +-----------------
    //                +------------------
    //               +-------------------
    //              +--------------------
    //             +---------------------
    //            +----------------------
    //           +-----------------------
    //          +------------------------
    //         +-------------------------
    //        +--------------------------
    //       +---------------------------
    //      +----------------------------
    //     +-----------------------------
    //    +------------------------------
    //   +-------------------------------
    //   ++++ ++++ ++++ ++++ ++AA AACC CCAA ADAC
    //   0000 0000 0000 0000 0011 1111 1111 1111 PAR
    //   0000 0000 0000 0000 0000 0000 0000 0100 SOR
    //   0000 0000 0000 0000 0000 0011 1100 0101 DIR
    IMM->io_regs[PORT_B].ppar = 0x00003FFF;
    IMM->io_regs[PORT_B].psor = 0x00000004;
    IMM->io_regs[PORT_B].pdir = 0x000003C5;
    IMM->io_regs[PORT_B].podr = 0x00000000;

    // Port C
    //             1111111111222222222233
    //   01234567890123456789012345678901
    //   ________________________________
    //                                  +
    //                                 +-
    //                                +--
    //                               +---
    //                              +----
    //                             +-----
    //                            +------
    //                           +-------
    //                          +--------
    //                         A--------- CLK10 (FCC1 Rx) [new board layout]
    //                        A---------- CLK11 (FCC1 Tx)
    //                       A----------- CLK12 (FCC1 Rx) [old board layout]
    //                      +------------
    //                     A------------- CLK14 (FCC2 Tx)
    //                    A-------------- CLK15 (FCC2 Rx)
    //                   +--------------- 
    //                  A---------------- SCC1 - CTS
    //                 A----------------- SCC1 - CD
    //                +------------------
    //               +-------------------
    //              +--------------------
    //             +---------------------
    //            +----------------------
    //           +-----------------------
    //          +------------------------
    //         +-------------------------
    //        +--------------------------
    //       +---------------------------
    //      +----------------------------
    //     +-----------------------------
    //    +------------------------------
    //   +-------------------------------
    //   ++++ ++++ ++++ ++AA +AA+ AAA+ ++++ ++++
    //   0000 0000 0000 0011 0110 ?1?0 0000 0000 PAR  (depending on board layout)
    //   0000 0000 0000 0000 0000 0000 0000 0000 SOR
    //   0000 0000 0000 0000 0000 0000 0000 0000 DIR
// #define SCC1_FULL_HANDSHAKE
#ifdef SCC1_FULL_HANDSHAKE
    IMM->io_regs[PORT_C].ppar = 0x00036000;
#else
    IMM->io_regs[PORT_C].ppar = 0x00006000;
#endif
    if (old_board_layout) {
        IMM->io_regs[PORT_C].ppar |= 0x00000C00;
    } else {
        IMM->io_regs[PORT_C].ppar |= 0x00000600;
    }
    IMM->io_regs[PORT_C].psor = 0x00000000;
    IMM->io_regs[PORT_C].pdir = 0x00000000;
    IMM->io_regs[PORT_C].podr = 0x00000000;

    // Port D
    //             1111111111222222222233
    //   01234567890123456789012345678901
    //   ________________________________
    //                                  A SCC1 - RxD
    //                                 D- SCC1 - TxD
    //                                C-- SCC1 - RTS
    //                               +---
    //                              +----
    //                             +-----
    //                            +------
    //                           +-------
    //                          +--------
    //                         +---------
    //                        +----------
    //                       +-----------
    //                      +------------
    //                     +-------------
    //                    +--------------
    //                   +---------------
    //                  +----------------
    //                 +-----------------
    //                +------------------
    //               +-------------------
    //              +--------------------
    //             +---------------------
    //            C---------------------- SMC1 - TxD
    //           A----------------------- SMC1 - RxD
    //          +------------------------
    //         +-------------------------
    //        +--------------------------
    //       +---------------------------
    //      +----------------------------
    //     +-----------------------------
    //    +------------------------------
    //   +-------------------------------
    //   ++++ ++++ AC++ ++++ ++++ ++++ ++++ +CDA
    //   0000 0000 1100 0000 0000 0000 0000 0111 PAR
    //   0000 0000 0000 0000 0000 0000 0000 0010 SOR
    //   0000 0000 0100 0000 0000 0000 0000 0110 DIR
#ifdef SCC1_FULL_HANDSHAKE
    IMM->io_regs[PORT_D].ppar = 0x00C00003;
    IMM->io_regs[PORT_D].psor = 0x00000002;
    IMM->io_regs[PORT_D].pdir = 0x00400006;
    IMM->io_regs[PORT_D].podr = 0x00000000;
#else
    IMM->io_regs[PORT_D].ppar = 0x00C00007;
    IMM->io_regs[PORT_D].psor = 0x00000002;
    IMM->io_regs[PORT_D].pdir = 0x00400002;
    IMM->io_regs[PORT_D].podr = 0x00000000;
#endif

    // Misc I/O bits
    IMM->io_regs[PORT_B].pdir |= 0x03000000;  // PB7 = Reset FCC1 PHY
                                              // PB6 = Reset FCC2 PHY

    IMM->io_regs[PORT_B].pdat |= 0x03000000;

    IMM->io_regs[PORT_C].pdir |= 0x3C000000;  // PC5 = FCC2 MDIO
                                              // PC4 = FCC2 MDC
                                              // PC3 = FCC1 MDIO
                                              // PC2 = FCC1 MDC
    IMM->io_regs[PORT_C].pdat |= 0x3C000000;

    // Clock steering
    IMM->cpm_mux_cmxuar = 0x00000000;        // Utopia address reg
    if (old_board_layout) {
        // Mux for FCC
        // --11 0111 --11 0101 ---- ---- ---- ----
        //   xx x                                   FCC1 - Tx clock 11
        //       xxx                                FCC1 - Rx clock 12
        //             xx x                         FCC2 - Tx clock 14
        //                 xxx                      FCC2 - Rx clock 15
        IMM->cpm_mux_cmxfcr = 0x37350000;
    } else {
        // Mux for FCC
        // --11 0101 --11 0101 ---- ---- ---- ----
        //   xx x                                   FCC1 - Tx clock 11
        //       xxx                                FCC1 - Rx clock 10
        //             xx x                         FCC2 - Tx clock 14
        //                 xxx                      FCC2 - Rx clock 15
        IMM->cpm_mux_cmxfcr = 0x35350000;
    }
    // Mux for SCCx
    // --00 0000 --00 1001 --01 0010 --01 1011 
    //   xx x                                   SCC1 - Rx clock BRG1
    //       xxx                                SCC1 - Tx clock BRG1
    //             xx x                         SCC2 - Rx clock BRG2
    //                 xxx                      SCC2 - Tx clock BRG2
    //                       xx x               SCC3 - Rx clock BRG3
    //                           xxx            SCC3 - Tx clock BRG3
    //                                 xx x     SCC4 - Rx clock BRG4
    //                                     xxx  SCC4 - Tx clock BRG4
    IMM->cpm_mux_cmxscr = 0x0009121B;
    // Mux for SMCx
    // --01 --01
    //   xx                                     SMC1 - clock on BRG7
    //        xx                                SMC2 - clock on BRG8
    IMM->cpm_mux_cmxsmr = 0x11;
#endif // CYGSEM_HAL_USE_ROM_MONITOR
    
    // Start up system I/O
    hal_if_init();

#ifndef CYGSEM_HAL_USE_ROM_MONITOR
#ifdef CYGHWR_HAL_POWERPC_RATTLER_PCI
    if ((IMM->clocks_sccr & 0x100) != 0) {
        CYG_WORD16 pci_cfg;

        HAL_WRITE_UINT32LE(&IMM->pci_cfg_addr, 0x80000004);
        HAL_WRITE_UINT16LE(&IMM->pci_cfg_data, 0);
        // Configure PCI address registers
        IMM->pcimsk1 = 0xC0000000;
        IMM->pcibr1 = 0x80000001;
        IMM->pcimsk0 = 0xFF800000;
        IMM->pcibr0 = 0x48000001;
        IMM->pci_gpcr = 0;
        IMM->pci_picmr1 = 0xF0FF0FE0;
        IMM->pci_picmr0 = 0xF0FF0FE0;
        // Now disable CFG_LOCK to free bus
        HAL_WRITE_UINT32LE(&IMM->pci_cfg_addr, 0x80000044);
        HAL_READ_UINT16LE(&IMM->pci_cfg_data, pci_cfg);
        pci_cfg &= ~0x20;  // Turn off CFG_LOCK
        HAL_WRITE_UINT32LE(&IMM->pci_cfg_addr, 0x80000044);
        HAL_WRITE_UINT16LE(&IMM->pci_cfg_data, pci_cfg);
        HAL_WRITE_UINT32LE(&IMM->pci_cfg_addr, 0x80000044);
        HAL_READ_UINT16LE(&IMM->pci_cfg_data, pci_cfg);
    } else {
        diag_printf("*** Warning: PCI not responding - SCCR: %x\n", IMM->clocks_sccr);
    }
#endif
#endif // CYGSEM_HAL_USE_ROM_MONITOR
}

//
// Cause the platform to reset
//
void
_rattler_reset(void)
{
    unsigned long hid0, int_state;

    // Need interrupts off to force checkstop
    HAL_DISABLE_INTERRUPTS(int_state);
    HAL_DISABLE_MACHINE_CHECK(int_state);
    IMM->clocks_rmr |= 0x01;  // Checkstop Reset Enable
    // Force a checkstop by turning on parity which is not implemented
    CYGARC_MFSPR(CYGARC_REG_HID0, hid0); 
    hid0 |= 0x30000000;
    CYGARC_MTSPR(CYGARC_REG_HID0, hid0); 
    diag_printf("...RESET\n");
    while (1) ;
}

//
// Display a value in the LEDs
// Note: the values used/returned by this function are positive
// i.e. a value of 0 is all LEDs off, 0x7 is all on
//
#define LED_SHIFT 29  // LEDs are in bits A0..A2
#define LED_MASK   7  // 3 bits total
int
_rattler_leds(int val)
{
    volatile t_PQ2IMM  *IMM = (volatile t_PQ2IMM *)CYGARC_IMM_BASE;
    unsigned int old_val = ~(IMM->io_regs[PORT_A].pdat >> LED_SHIFT) & LED_MASK;
    unsigned int new_val = (old_val & ~(LED_MASK<<LED_SHIFT)) | ((~val&LED_MASK)<<LED_SHIFT);

    IMM->io_regs[PORT_A].pdat = new_val;
    return old_val;
}

// EOF hal_aux.c
