//=============================================================================
//
//      sh4_scif.h
//
//      Simple driver for the sh4 Serial Communication Interface with FIFO
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
// Contributors:Haruki Kashiwaya
// Date:        2000-08-09
// Description: Simple driver for the SH Serial Communication Interface
//              The driver can be used for the SCIF modules.
//              Clients of this file can configure the behavior with:
//              CYGNUM_SCIF_PORTS: number of SCI ports
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef CYGNUM_HAL_SH_SH4_SCIF_PORTS

//--------------------------------------------------------------------------
// Exported functions

externC cyg_uint8 cyg_hal_plf_scif_getc(void* __ch_data);
externC void cyg_hal_plf_scif_putc(void* __ch_data, cyg_uint8 c);
void cyg_hal_plf_scif_init(int scif_index, int comm_index, 
                           int rcv_vect, cyg_uint8* base);

#ifdef CYGPRI_HAL_SH_SH4_SCIF_PRIVATE

//--------------------------------------------------------------------------
// SCIF register offsets
#define _REG_SCSMR  0x00
#define _REG_SCBRR  0x04
#define _REG_SCSCR  0x08
#define _REG_SCFTDR 0x0c
#define _REG_SCFSR  0x10
#define _REG_SCFRDR 0x14
#define _REG_SCFCR  0x18
#define _REG_SCFDR  0x1c
#define _REG_SCSPTR 0x20
#define _REG_SCLSR  0x24

//--------------------------------------------------------------------------

typedef struct {
    cyg_uint8* base;
    cyg_int32 msec_timeout;
    int isr_vector;
    cyg_uint32 baud_rate;
} channel_data_t;

//--------------------------------------------------------------------------

#if !defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG)
// This one should only be used by old-stub compatibility code!
externC void cyg_hal_plf_scif_init_channel(channel_data_t* chan);
#warning "You should not be using anything but vv diag"
#endif

#endif // CYGPRI_HAL_SH_SH4_SCIF_PRIVATE

#endif // CYGNUM_HAL_SH_SH4_SCIF_PORTS
//-----------------------------------------------------------------------------
// end of sh4_scif.h
