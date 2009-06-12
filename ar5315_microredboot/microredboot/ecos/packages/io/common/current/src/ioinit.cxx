//==========================================================================
//
//      io/iosys.c
//
//      I/O Subsystem + Device Table support
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
// Author(s):   gthomas
// Contributors:  gthomas
// Date:        1999-02-04
// Purpose:     Device Table
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/io/io.h>
#include <cyg/io/devtab.h>

// This is a dummy class just so we can execute the I/O package 
// initialization at it's proper priority

externC void cyg_io_init(void);

class cyg_io_init_class {
public:
    cyg_io_init_class(void) { 
        cyg_io_init();
    }
};

// And here's an instance of the class just to make the code run
static cyg_io_init_class _cyg_io_init CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_IO);

// Define table boundaries
CYG_HAL_TABLE_BEGIN( __DEVTAB__, devtab );
CYG_HAL_TABLE_END( __DEVTAB_END__, devtab );
