//==========================================================================
//
//      common/delete.cxx
//
//      Default delete implementation
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
// Author(s):   jlarmour
// Contributors:  jlarmour
// Date:        1999-08-13
// Purpose:     Gives a default delete operation if the C library isn't present
// Description: This file supplies a definition of the default delete
//              operation (aka __builtin_delete and __builtin_vec_delete)
//              for use when the normal delete can't be used - normally when
//              the C library is not present
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/infra.h>

#include <cyg/infra/cyg_type.h>

// see the description comment in infra.cdl for
// CYGFUN_INFRA_EMPTY_DELETE_FUNCTIONS

#ifdef CYGFUN_INFRA_EMPTY_DELETE_FUNCTIONS
// then define these empty functions:

void operator delete(void *x) throw()
{
    CYG_EMPTY_STATEMENT;
}

void operator delete[](void *x) throw()
{
    CYG_EMPTY_STATEMENT;
}

#endif // CYGFUN_INFRA_EMPTY_DELETE_FUNCTIONS

// EOF delete.cxx
