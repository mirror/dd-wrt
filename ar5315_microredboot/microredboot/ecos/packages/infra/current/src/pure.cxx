//==========================================================================
//
//      pure.cxx
//
//      g++ support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Bart Veer
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   bartv
// Date:        2003-03-27
// Purpose:     provide a __cxa_pure_virtual() function
// Description: g++ v3.x generates references to a function __cxa_pure_virtual()
//              whenever the code contains pure virtual functions. This is a
//              placeholder used while constructing an object, and the function
//              should never actually get called. There is a default
//              implementation in libsupc++ but that has dependencies on I/O
//              and so on, unwanted in a minimal eCos environment. Instead we
//              want a minimal __cxa_pure_virtual().
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/infra.h>
#include <cyg/infra/cyg_ass.h>

extern "C" void
__cxa_pure_virtual(void)
{
    CYG_FAIL("attempt to use a virtual function before object has been constructed");
    for ( ; ; );
}

// EOF pure.cxx
