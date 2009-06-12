//=============================================================================
//
//      mod_regs_cpg.h
//
//      CPG (clock pulse generator) Module register definitions
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
// Date:        2002-01-16
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//--------------------------------------------------------------------------
// Oscillator control registers
#define CYGARC_REG_FMR                  0xfffffe90

#define CYGARC_REG_FMR_PLL2ST           0x80
#define CYGARC_REG_FMR_PLL1ST           0x40
#define CYGARC_REG_FMR_CKIOST           0x20
#define CYGARC_REG_FMR_FR3              0x08
#define CYGARC_REG_FMR_FR2              0x04
#define CYGARC_REG_FMR_FR1              0x02
#define CYGARC_REG_FMR_FR0              0x01

// Translate various CDL clock configurations to register equivalents
// for the various CPG versions
#if   (CYGARC_SH_MOD_CPG == 1) // ---------------------------- V1

// For now, platform CDL must provide it
#ifndef CYGHWR_HAL_SH_SH2_FMR_INIT
# error "No FMR_INIT value provided"
#endif

#define CYGARC_REG_FMR_INIT CYGHWR_HAL_SH_SH2_FMR_INIT

#elif (CYGARC_SH_MOD_CPG == 2)

#else

# error "Unsupported CPG version"

#endif

// Init value
//#define CYGARC_REG_FMR_INIT ()
