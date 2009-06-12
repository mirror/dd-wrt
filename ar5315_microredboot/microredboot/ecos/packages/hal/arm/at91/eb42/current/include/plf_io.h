#ifndef CYGONCE_HAL_PLF_IO_H
#define CYGONCE_HAL_PLF_IO_H
//=============================================================================
//
//      plf_io.h
//
//      EB42 board specific registers
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett <nickg@calivar.com>
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
// Contributors: tdrury, nickg
// Date:        2002-06-22
// Purpose:     Atmel EB42 board specific registers
// Description:
// Usage:       #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================
// On-chip device base addresses

#define AT91_PIOA       0xFFFEC000
#define AT91_PIOB       0xFFFF0000

#define AT91_PIO        AT91_PIOB

#define AT91_USART0     0xFFFC0000
#define AT91_USART1     0xFFFC4000

#define AT91_TCB0       0xFFFD0000
#define AT91_TCB1       0xFFFD4000

#define AT91_TC         AT91_TCB0

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_HAL_PLF_IO_H
