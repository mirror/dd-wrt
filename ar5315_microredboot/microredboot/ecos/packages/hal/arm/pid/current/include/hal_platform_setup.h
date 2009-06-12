#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H

/*=============================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL (assembly code)
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
// Date:        1999-04-13
// Purpose:     ARM/PID platform specific support routines
// Description: 
// Usage:       #include <cyg/hal/hal_platform_setup.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/


// Define macro used to diddle the LEDs during early initialization.
// Can use r0+r1. Argument in \x.
// Control the LEDs PP0-PP3. This requires the jumpers on pins 9-16 to
// be set on LK11 in order to be visible. Otherwise the parallel port
// data pins are diddled instead.
#ifdef CYGHWR_HAL_ARM_PID_DIAG_LEDS
#define CYGHWR_LED_MACRO                                                  \
        ldr     r0,=0x0d800040                                           ;\
        mov     r1, #((15 & (\x)) << 4)                                  ;\
        strb    r1, [r0]                                                 ;
#endif

#define MEM_RESET         0x0B000020  // Write to this location to enable RAM

#define PLATFORM_SETUP1                                 \
        ldr     r0,=10f                                ;\
        mov     pc,r0                                  ;\
10:	ldr	r0,=MEM_RESET	/* enable RAM */       ;\
	str	r0,[r0]

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
