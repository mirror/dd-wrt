#ifndef CYGONCE_HAL_HAL_SPD_H
#define CYGONCE_HAL_HAL_SPD_H

//=============================================================================
//
//      hal_spd.h
//
//      HAL header for SDRAM Serial Presence Detect support.
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
// Author(s):   msalter
// Contributors:msalter
// Date:        2002-01-17
// Purpose:     Generic HAL SPD header.
// Usage:       #include <cyg/hal/hal_spd.h>
// Description: This header provides byte numbers and bit definitions for
//              serial EEPROM containing SDRAM module information.
//                           
//####DESCRIPTIONEND####
//
//=============================================================================

// Commonly used bytes
#define	SPD_BANKCNT	    5	// number of module banks
#define	SPD_CONFIG	   11	// DIMM configuration type (Parity or not, EEC)
#define	SPD_REFRESH        12 	// Referesh rate
#define	SPD_SDRAM_WIDTH    13 	// DRAM width
#define	SPD_MOD_ATTRIB     21 	// DRAM module attribute
#define	SPD_BANKSZ	   31	// module bank density
#define	SPD_CHECKSUM	   63 	// checksum for bytes 0-62

// SPD_MOD_ATTRIB bits
#define SPD_ATTRIB_BUF_CTL 0x01 // Buffered Addr/Control inputs
#define SPD_ATTRIB_REG_CTL 0x02 // Registered Addr/Control inputs
#define SPD_ATTRIB_PLL     0x04 // On-card PLL
#define SPD_ATTRIB_BUF_DQ  0x08 // Buffered DQMB inputs
#define SPD_ATTRIB_REG_DQ  0x10 // Registered DQMB inputs
#define SPD_ATTRIB_DIFF    0x20 // Differential clock input
#define SPD_ATTRIB_RRA     0x40 // Redundant Row Address

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_HAL_SPD_H
// End of hal_spd.h
