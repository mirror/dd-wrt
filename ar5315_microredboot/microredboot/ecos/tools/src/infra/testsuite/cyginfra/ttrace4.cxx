//==========================================================================
//
//      ttrace4.cxx
//
//      Trace test case                                                                
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1999, 2000 Red Hat, Inc.
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
// Date:                1999-01-06
// Purpose:
// Description:         Do lots of tracing and generate a dump. The test
//                      harness checks that there is lots of output and
//                      that the first line matches expectations.
//
//####DESCRIPTIONEND####
//==========================================================================


#define CYGDBG_USE_ASSERTS
#define CYGDBG_USE_TRACING
#define CYGDBG_INFRA_DEBUG_FUNCTION_REPORTS
#include <cyg/infra/testcase.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/cyg_ass.h>
#include <cstdlib>

void
dummy(void)
{
    CYG_REPORT_FUNCNAME("dummy");
    CYG_REPORT_RETURN();
}

int
main(int argc, char** argv)
{
    for (int i = 0; i < 100000; i++) {
        CYG_TRACE0(true, "no argument here");
        dummy();
    }
    CYG_TRACE0(true, "Goodbye and thanks for all the fish");
    CYG_FAIL("intentional");
}
