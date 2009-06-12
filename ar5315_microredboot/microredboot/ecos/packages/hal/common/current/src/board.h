#ifndef CYGONCE_HAL_BOARD_H
#define CYGONCE_HAL_BOARD_H

//=============================================================================
//
//      board.h
//
//      libstub board.h file for eCos HAL
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
// Date:        1999-02-12
// Purpose:     libstub board.h file for eCos HAL
// Description: This file simplifies libstub integration. The board.h file
//              for eCos simply includes the hal_stub.h file which defines
//              the necessary architecture and platform information for
//              any of the possible eCos target configurations.
//              
//####DESCRIPTIONEND####
//
//=============================================================================

// Define __ECOS__; allows all eCos specific additions to be easily identified.
#define __ECOS__

#include <cyg/hal/hal_stub.h>

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#endif

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_BOARD_H
// End of board.h
