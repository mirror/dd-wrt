#ifndef CYGONCE_HAL_CMA230_H
#define CYGONCE_HAL_CMA230_H

/*=============================================================================
//
//      hal_cma230.h
//
//      HAL Support for Kernel Diagnostic Routines
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         1999-04-19
// Purpose:      Cogent CMA230 hardware description
// Description:
// Usage:        #include <cyg/hal/hal_cma230.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

// Note: these defintions match the documentation, thus no attempt is made
// to sanitise (mangle) the names.  Also, care should be taken to keep this
// clean for use in assembly code (no "C" constructs).

#define CMA230_ISR  0x0F600000  // Interrupt source register    - Read only
#define CMA230_CLR  0x0F600008  // Clear interrupt source       - Write only
#define CMA230_IMRr 0x0F600010  // Interrupt mask register      - Read only
#define CMA230_IMRw 0x0F600018  // Interrupt mask register      - Write only
#define CMA230_ACK1 0x0F600020  // Interrupt acknowledge slot 1 - Read only
#define CMA230_ACK2 0x0F600028  // Interrupt acknowledge slot 2 - Read only
#define CMA230_ACK3 0x0F600030  // Interrupt acknowledge slot 3 - Read only

#define CMA230_TC_COUNT     0x0F700020  // Timer value   - 16 bits - Read only
#define CMA230_TC_PRELOAD   0x0F700028  // Timer preload - 16 bits - Write only
#define CMA230_TC_CLEAR     0x0F700030  // Timer clear             - Write only
#define CMA230_TC_ENABLE    0x0F700038  // Timer enable/start      - Write only

// Motherboard definitions

#define CMA101_DUARTB      0x0E900000  // Base address
#define CMA101_DUARTB_RHR  0x0E900000  // Receive holding register
#define CMA101_DUARTB_THR  0x0E900000  // Transmit holding register
#define CMA101_DUARTB_LBR  0x0E900000  // Low byte of baud rate
#define CMA101_DUARTB_IER  0x0E900008  // Interrupt enable
#define CMA101_DUARTB_HBR  0x0E900008  // High byte of baud rate
#define CMA101_DUARTB_ISR  0x0E900010  // Interrupt status
#define CMA101_DUARTB_FCTL 0x0E900010  // FIFO control
#define CMA101_DUARTB_LCTL 0x0E900018  // Line control
#define CMA101_DUARTB_MCTL 0x0E900020  // Modem control
#define CMA101_DUARTB_LSR  0x0E900028  // Line status
#define CMA101_DUARTB_MSR  0x0E900030  // Modem status
#define CMA101_DUARTB_SCR  0x0E900038  // Scratch

#define CMA101_DUARTA      0x0E900040  // Base address
#define CMA101_DUARTA_RHR  0x0E900040  // Receive holding register
#define CMA101_DUARTA_THR  0x0E900040  // Transmit holding register
#define CMA101_DUARTA_LBR  0x0E900040  // Low byte of baud rate
#define CMA101_DUARTA_IER  0x0E900048  // Interrupt enable
#define CMA101_DUARTA_HBR  0x0E900048  // High byte of baud rate
#define CMA101_DUARTA_ISR  0x0E900050  // Interrupt status
#define CMA101_DUARTA_FCTL 0x0E900050  // FIFO control
#define CMA101_DUARTA_LCTL 0x0E900058  // Line control
#define CMA101_DUARTA_MCTL 0x0E900060  // Modem control
#define CMA101_DUARTA_LSR  0x0E900068  // Line status
#define CMA101_DUARTA_MSR  0x0E900070  // Modem status
#define CMA101_DUARTA_SCR  0x0E900078  // Scratch

/*---------------------------------------------------------------------------*/
/* end of hal_cma230.h                                                         */
#endif /* CYGONCE_HAL_CMA230_H */
