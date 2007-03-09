//==========================================================================
//
//      testcase.cxx
//
//      Host side implementation of the test support routines.
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
// Date:                1998-01-01
// Purpose:
// Description:
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cstdio>
#include <cstdlib>

#include <cyg/infra/testcase.h>

// Initialization is a no-op for the host side testing infrastructure.
// Only batch programs are likely to use these testing facilities so
// it is safe to assume that stdio is available.

externC void
cyg_test_init(void)
{
}

// This simply implements the current interface, warts and all.
// It is necessary to keep track of any failures or invalid
// calls.
static int failures = 0;

externC void
cyg_test_output(Cyg_test_code status, const char* msg, int line_number, const char* file)
{
    if (CYGNUM_TEST_FAIL == status)
        failures++;
    
    if (0 == msg)
        msg = "";
    if (0 == file)
        file = "";
    
    if (CYGNUM_TEST_FAIL == status) {
        printf("FAIL:<%s> Line: %d, File: %s\n", msg, line_number, file);
    } else {
        printf("%s:<%s>\n",
               (CYGNUM_TEST_PASS == status)   ? "PASS" :
               (CYGNUM_TEST_EXIT == status)   ? "EXIT" :
               (CYGNUM_TEST_INFO == status)   ? "INFO" :
               (CYGNUM_TEST_GDBCMD == status) ? "GDBCMD" :
               (CYGNUM_TEST_NA == status)     ? "NA" : "UNKNOWN STATUS",
               msg);
    }
  
}   

externC void
cyg_test_exit(void)
{
    exit( (0 < failures) ? EXIT_FAILURE : EXIT_SUCCESS );
}
