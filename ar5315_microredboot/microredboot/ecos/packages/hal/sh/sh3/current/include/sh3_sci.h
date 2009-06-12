//=============================================================================
//
//      sh3_sci.h
//
//      Simple driver for the SH Serial Communication Interface (SCI)
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
// Author(s):   jskov
// Contributors:jskov
// Date:        1999-05-17
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef CYGNUM_HAL_SH_SH3_SCI_PORTS

//--------------------------------------------------------------------------
// Exported functions

externC cyg_uint8 cyg_hal_plf_sci_getc(void* __ch_data);
externC void cyg_hal_plf_sci_putc(void* __ch_data, cyg_uint8 c);
externC void cyg_hal_plf_sci_init(int sci_index, int comm_index, 
                                  int rcv_vect, cyg_uint8* base);


#ifdef CYGPRI_HAL_SH_SH3_SCI_PRIVATE

//--------------------------------------------------------------------------
// SCI register offsets
#if (CYGARC_SH_MOD_SCI >= 2)
# define _REG_SCSPTR             -0x4 // serial port register
#endif
#define _REG_SCSMR                0x0 // serial mode register
#define _REG_SCBRR                0x2 // bit rate register
#define _REG_SCSCR                0x4 // serial control register
#define _REG_SCTDR                0x6 // transmit data register
#define _REG_SCSSR                0x8 // serial status register
#define _REG_SCRDR                0xa // receive data register

//--------------------------------------------------------------------------

typedef struct {
    cyg_uint8* base;
    cyg_int32 msec_timeout;
    int isr_vector;
} channel_data_t;

//--------------------------------------------------------------------------

#if !defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG)
// This one should only be used by old-stub compatibility code!
externC void cyg_hal_plf_sci_init_channel(channel_data_t* chan);
#warning "You should not be using anything but vv diag"
#endif

#endif // CYGPRI_HAL_SH_SH3_SCI_PRIVATE

#endif // CYGNUM_HAL_SH_SH3_SCI_PORTS
//-----------------------------------------------------------------------------
// end of sh3_sci.h
