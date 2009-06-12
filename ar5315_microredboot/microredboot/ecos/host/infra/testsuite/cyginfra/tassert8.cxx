//==========================================================================
//
//      tassert8.cxx
//
//      Assertion test case                                                                
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####                                             
//
// Author(s):           bartv
// Contributors:        bartv
// Date:                1998-12-23
// Purpose:
// Description:         This routine causes a basic assertion, including a
//                      number of callbacks. It is up to the test driver
//                      to analyse the results of this and take appropriate
//                      action.
//
//####DESCRIPTIONEND####
//==========================================================================

#define CYG_DECLARE_HOST_ASSERTION_SUPPORT
#define CYGDBG_USE_ASSERTS
#define CYGDBG_INFRA_DEBUG_PRECONDITIONS
#define CYGDBG_INFRA_DEBUG_POSTCONDITIONS
#define CYGDBG_INFRA_DEBUG_LOOP_INVARIANTS
#define CYGDBG_INFRA_DEBUG_INVARIANTS

#include <cyg/infra/testcase.h>
#include <cyg/infra/cyg_ass.h>

extern "C"
void
callback1(void (*outputfn)(const char*))
{
    // This callback does nothing
}

extern "C"
void
callback2(void (*outputfn)(const char*))
{
    // This callback outputs a number of lines of data.
    for (int i = 0; i < 10; i++) {
        (*outputfn)("callback2 output\n");
    }
}

int
main(int argc, char** argv)
{
    cyg_assert_install_failure_callback("callback1", &callback1);
    cyg_assert_install_failure_callback("callback2", &callback2);
    CYG_FAIL("it seemed like a good idea at the time");
}

