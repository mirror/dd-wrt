#ifndef CYGONCE_HAL_PLF_IO_H
#define CYGONCE_HAL_PLF_IO_H
//=============================================================================
//
//      plf_io.h
//
//      EB40A board specific registers
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
// Author(s):   tkoeller
// Contributors: tdrury
// Date:        2002-06-22
// Purpose:     Atmel EB40A board specific registers
// Description:
// Usage:       #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

// these io pins are for the LEDs.
#define EB40A_LED1 0x00000008
#define EB40A_LED2 0x00000010
#define EB40A_LED3 0x00000020
#define EB40A_LED4 0x00000040
// these PIOs double as TCLK1, TIOA1, TIOB1, TCLK2 respectively.
#define EB40A_LED5 0x00010000
#define EB40A_LED6 0x00020000
#define EB40A_LED7 0x00040000
#define EB40A_LED8 0x00080000

#define EB40A_LED_ALL 0x00f00078

// Push buttons. Note that these are connected to interrupt lines.
#define EB40A_SW1 0x00001000
#define EB40A_SW2 0x00000100
#define EB40A_SW3 0x00000002
#define EB40A_SW4 0x00000004

// Push buttons. Note that these are connected to interrupt lines.
#define EB40A_SW1 0x00001000 
#define EB40A_SW2 0x00000100 
#define EB40A_SW3 0x00000002
#define EB40A_SW4 0x00000004

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_HAL_PLF_IO_H
