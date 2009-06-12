#ifndef CYGONCE_DEVS_MBX_ETH_INL
#define CYGONCE_DEVS_MBX_ETH_INL
//==========================================================================
//
//      mbx_eth.inl
//
//      Hardware specifics for Motorola MBX ethernet support
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2002-11-19
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#define _get_led()  
#define _set_led(v) 

#define LED_TxACTIVE  7
#define LED_RxACTIVE  6
#define LED_IntACTIVE 5

#if 0
// Fetch ESA from on-board EEPROM
extern int _mbx_fetch_VPD(int, void *, int);
#define QUICC_ETH_FETCH_ESA(_ok_)                              \
     _ok_ = _mbx_fetch_VPD(VPD_ETHERNET_ADDRESS, enaddr, sizeof(enaddr));
#endif

// Reset/enable any external hardware
#define QUICC_ETH_ENABLE()                                     \
    *MBX_CTL1 = MBX_CTL1_ETEN | MBX_CTL1_TPEN;  /* Enable ethernet, TP mode */


// Port layout - uses SCC1
#define QUICC_ETH_PA_RXD            0x0001  // Rx Data on Port A
#define QUICC_ETH_PA_TXD            0x0002  // Tx Data on Port A
#define QUICC_ETH_PA_Tx_CLOCK       0x0200  // Tx Clock = CLK2
#define QUICC_ETH_PA_Rx_CLOCK       0x0800  // Rx Clock = CLK4
#define QUICC_ETH_PC_Tx_ENABLE      0x0001  // Tx Enable (TENA)
#define QUICC_ETH_PC_COLLISION      0x0010  // Collision detect
#define QUICC_ETH_PC_Rx_ENABLE      0x0020  // Rx Enable (RENA)
#define QUICC_ETH_SICR_MASK         0x00FF  // SI Clock Route - important bits
#define QUICC_ETH_SICR_ENET  (7<<3)|(5<<0)  //   Rx=CLK4, Tx=CLK2
#define QUICC_ETH_SICR_ENABLE       0x0040  // Enable SCC1 to use NMSI
#define QUICC_ETH_INT               CYGNUM_HAL_INTERRUPT_CPM_SCC1
#define QUICC_ETH_SCC               0       // SCC1
#define QUICC_CPM_SCCx              QUICC_CPM_SCC1

#define MBX_CTL1   (cyg_uint8 *)0xFA100000  // System control register
#define MBX_CTL1_ETEN                 0x80  // 1 = Enable ethernet tranceiver
#define MBX_CTL1_ELEN                 0x40  // 1 = Enable ethernet loopback
#define MBX_CTL1_EAEN                 0x20  // 1 = Auto select ethernet interface
#define MBX_CTL1_TPEN                 0x10  // 0 = AUI, 1 = TPI
#define MBX_CTL1_FDDIS                0x08  // 1 = Disable full duplex (if TP mode)


#endif  // CYGONCE_DEVS_MBX_ETH_INL
// ------------------------------------------------------------------------
