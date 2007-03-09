//{{{  Banner                                           

//============================================================================
//
//      checkdata.cxx
//
//      Host side implementation of the check_func() and check_data()
//      utilities.
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
//
// This file is part of the eCos host tools.
//
// This program is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
// more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ----------------------------------------------------------------------------
//                                                                          
//####COPYRIGHTEND####
//============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   bartv
// Contact(s):  bartv
// Date:        1998/12/02
// Version:     0.01
// Purpose:     To provide a host-side implementation of two of the
//              eCos assertion facilities. facilities.
//
//####DESCRIPTIONEND####
//============================================================================

//}}}
//{{{  #include's                                       

#include "cyg/infra/cyg_ass.h"

//}}}

// -------------------------------------------------------------------------
// It is convenient to have cyg_check_data_ptr() and cyg_check_func_ptr()
// in a separate file from cyg_assert_fail(). That way users can override
// cyg_assert_fail() itself in application code without losing access to
// the check_data and check_func routines.
//
// The actual implementation is very limited. It might be possible to
// do better on specific platforms.

extern "C" cyg_bool
cyg_check_data_ptr( void * ptr )
{
    return 0 != ptr;
}

extern "C" cyg_bool
cyg_check_func_ptr ( void (*ptr)(void) )
{
    return 0 != ptr;
}
