#ifndef CYGONCE_HAL_HAL_DIAG_H
#define CYGONCE_HAL_HAL_DIAG_H

/*=============================================================================
//
//      hal_diag.h
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
// Author(s):   yoshinori sato
// Contributors:        yoshinori sato
// Date:        2002-02-14
// Purpose:     HAL Support for Kernel Diagnostic Routines
// Description: Diagnostic routines for use during kernel development.
// Usage:       #include <cyg/hal/hal_diag.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>

externC void hal_diag_init(void);
externC void hal_diag_write_char(cyg_uint8 c);
externC void hal_diag_read_char(cyg_uint8 *c);

/*---------------------------------------------------------------------------*/

#define HAL_DIAG_INIT()          hal_diag_init()

#define HAL_DIAG_WRITE_CHAR(_c_) hal_diag_write_char(_c_)

#define HAL_DIAG_READ_CHAR(_c_)     hal_diag_read_char(&_c_)

/*---------------------------------------------------------------------------*/
/* end of hal_diag.h                                                         */
#endif /* CYGONCE_HAL_HAL_DIAG_H */
