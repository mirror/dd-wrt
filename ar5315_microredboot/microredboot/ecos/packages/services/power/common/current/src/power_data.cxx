//==========================================================================
//
//      power_data.cxx
//
//      Initialization and table support for power management.
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
// Author(s):    bartv
// Contributors: bartv
// Date:         2001-06-12
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/power.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_tables.h>

// Run-time package initialization. This is only required when the
// power management package runs its own thread. It involves a dummy
// class plus a single instance of that class. Initialization is
// scheduled for after low-level device driver init, kernel init, and
// I/O subsystem init. That way all device drivers should be up and
// running already.

#ifdef CYGPKG_POWER_THREAD
externC void power_init(void);

class power_init_class {
public:
    power_init_class(void) { 
        power_init();
    }
};

static power_init_class _power_init CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_IO + 1);
#endif

// Define the table for power controllers.
CYG_HAL_TABLE_BEGIN( __POWER__, power );
CYG_HAL_TABLE_END( __POWER_END__, power );
