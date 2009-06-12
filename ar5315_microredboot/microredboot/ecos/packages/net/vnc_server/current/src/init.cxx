//==========================================================================
//
//      init.cxx
//
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):    Chris Garry <cgarry@sweeneydesign.co.uk>
// Contributors:
// Date:         2003-08-22
// Purpose:
// Description:  Constructor for VNC Server
//
//####DESCRIPTIONEND####
//
//========================================================================*/


#include <pkgconf/system.h>
#include <pkgconf/isoinfra.h>
#include <cyg/infra/cyg_ass.h>         // assertion macros

/* ================================================================= */

__externC void cyg_vnc_server_startup(void);

/* ================================================================= */
/* Initialization object
 */

class Cyg_Vnc_Server_Init_Class
{
public:
    Cyg_Vnc_Server_Init_Class();
};

/* ----------------------------------------------------------------- */
/* Static initialization object instance. The constructor is
 * prioritized to run at the same time as any filesystem constructors.
 */
static Cyg_Vnc_Server_Init_Class vnc_initializer CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_IO_FS);

/* ----------------------------------------------------------------- */
/* Constructor, just calls the startup routine.
 */

Cyg_Vnc_Server_Init_Class::Cyg_Vnc_Server_Init_Class()
{
    cyg_vnc_server_startup();
}

/* ----------------------------------------------------------------- */
/* end of init.cxx                                                   */
