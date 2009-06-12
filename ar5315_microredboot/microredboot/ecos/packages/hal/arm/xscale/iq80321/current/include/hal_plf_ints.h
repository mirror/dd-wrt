#ifndef CYGONCE_HAL_PLF_INTS_H
#define CYGONCE_HAL_PLF_INTS_H
//==========================================================================
//
//      hal_plf_ints.h
//
//      HAL Platform Interrupt support
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter
// Date:         2001-12-03
// Purpose:      Define Interrupt support
// Description:  The interrupt details for a specific platform is defined here.
// Usage:
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// start with variant ints
#include CYGBLD_HAL_VAR_INTS_H

#ifdef CYGSEM_HAL_ARM_IQ80321_FAB_C
#define CYGNUM_HAL_INTERRUPT_ETHERNET CYGNUM_HAL_INTERRUPT_XINT2
#define CYGNUM_HAL_INTERRUPT_UART  CYGNUM_HAL_INTERRUPT_XINT3
#else
#define CYGNUM_HAL_INTERRUPT_ETHERNET CYGNUM_HAL_INTERRUPT_XINT0
#define CYGNUM_HAL_INTERRUPT_UART  CYGNUM_HAL_INTERRUPT_XINT1
#endif

#endif // CYGONCE_HAL_PLF_INTS_H
