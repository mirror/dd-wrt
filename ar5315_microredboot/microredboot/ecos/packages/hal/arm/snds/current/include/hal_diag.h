//==========================================================================
//
//      hal_diag.h
//
//      
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
// Author(s):    gthomas
// Contributors: gthomas, jskov
//               Grant Edwards <grante@visi.com>
// Date:         2001-07-31
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#ifndef CYGONCE_HAL_DIAG_H
#define CYGONCE_HAL_DIAG_H

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#if defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG)

#include <cyg/hal/hal_if.h>

#define HAL_DIAG_INIT() hal_if_diag_init()
#define HAL_DIAG_WRITE_CHAR(_c_) hal_if_diag_write_char(_c_)
#define HAL_DIAG_READ_CHAR(_c_) hal_if_diag_read_char(&_c_)

//// Not the best place for this, but ...
externC void hal_delay_us(cyg_int32 usecs);
#define HAL_DELAY_US(n)          hal_delay_us(n);

#else  // old way of doing diagnostic I/O

externC void hal_diag_init(void);
externC void hal_diag_write_char(char c);
externC void hal_diag_read_char(char *c);

#define HAL_DIAG_INIT() hal_diag_init()
#define HAL_DIAG_WRITE_CHAR(_c_) hal_diag_write_char(_c_)
#define HAL_DIAG_READ_CHAR(_c_) hal_diag_read_char(&_c_)

#endif // defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG)
#endif /* CYGONCE_HAL_DIAG_H */
