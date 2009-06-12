#ifndef CYGONCE_DEVS_ADDER_ETH_INL
#define CYGONCE_DEVS_ADDER_ETH_INL
//==========================================================================
//
//      adder_eth.inl
//
//      Hardware specifics for A&M Adder ethernet support
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


extern int  _adder_get_leds(void);
extern void _adder_set_leds(int);
extern bool _adder_reset_phy(void);

#define _get_led()  _adder_get_leds()
#define _set_led(v) _adder_set_leds(v)

#define LED_TxACTIVE  2
#define LED_RxACTIVE  1

// Reset the PHY - analagous to hardware reset
#define QUICC_ETH_RESET_PHY()                                   \
    if (!_adder_reset_phy()) {                                  \
        diag_printf("Can't reset PHY or get link\n");           \
    }

// Port layout - uses SCC2
#define QUICC_ETH_INT               CYGNUM_HAL_INTERRUPT_CPM_SCC2
#define QUICC_ETH_SCC               1       // SCC2
#define QUICC_CPM_SCCx              QUICC_CPM_SCC2

// Fixed bits
#define QUICC_ETH_PA_RXD            0x0004  // Rx Data on Port A
#define QUICC_ETH_PA_TXD            0x0008  // Tx Data on Port A
#define QUICC_ETH_PC_COLLISION      0x0040  // Collision detect
#define QUICC_ETH_PC_Rx_ENABLE      0x0080  // Rx Enable (RENA)

// These depend on how the PHY is wired to the CPU
#define QUICC_ETH_PA_Tx_CLOCK       0x0200  // Tx Clock = CLK2
#define QUICC_ETH_PA_Rx_CLOCK       0x0800  // Rx Clock = CLK4
#define QUICC_ETH_SICR_MASK         0xFF00  // SI Clock Route - important bits
#define QUICC_ETH_SICR_ENET  (7<<11)|(5<<8) //   Rx=CLK4, Tx=CLK2
#define QUICC_ETH_SICR_ENABLE       0x4000  // Enable SCC2 to use NMSI

// The TENA signal can appear on either port B or C
//#define QUICC_ETH_PC_Tx_ENABLE      0x0002  // Tx Enable (TENA)
#define QUICC_ETH_PB_Tx_ENABLE      0x2000  // Tx Enable (TENA)


#endif  // CYGONCE_DEVS_ADDER_ETH_INL
// ------------------------------------------------------------------------
