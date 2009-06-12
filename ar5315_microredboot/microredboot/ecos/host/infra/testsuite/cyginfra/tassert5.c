/* ==========================================================================
 *
 *      tassert5.c
 *
 *      Assertion test case                                                                
 *
 *==========================================================================
 *####COPYRIGHTBEGIN####
 *                                                                          
 * ----------------------------------------------------------------------------
 * Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
 *
 * This file is part of the eCos host tools.
 *
 * This program is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU General Public License as published by the Free 
 * Software Foundation; either version 2 of the License, or (at your option) 
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * ----------------------------------------------------------------------------
 *                                                                          
 *####COPYRIGHTEND####
 *==========================================================================
 *#####DESCRIPTIONBEGIN####                                             
 *
 * Author(s):           bartv
 * Contributors:        bartv
 * Date:                1998-11-27
 * Purpose:
 * Description:         A C equivalent of tassert2.cxx
 *
 *####DESCRIPTIONEND####
 *==========================================================================
 */

#define CYGDBG_USE_ASSERTS
#define CYGDBG_INFRA_DEBUG_PRECONDITIONS
#define CYGDBG_INFRA_DEBUG_POSTCONDITIONS
#define CYGDBG_INFRA_DEBUG_LOOP_INVARIANTS
#define CYGDBG_INFRA_DEBUG_INVARIANTS

#include <cyg/infra/testcase.h>
#include <cyg/infra/cyg_ass.h>
#include <stdlib.h>


static const char message[] = "This should never be seen.";

int main(int argc, char** argv)
{
    CYG_ASSERT( true, message);
    CYG_ASSERTC(true);
        
    CYG_CHECK_DATA_PTR( &argc, message);
    CYG_CHECK_FUNC_PTR( &main, message);
    CYG_CHECK_DATA_PTRC(&argc);
    CYG_CHECK_FUNC_PTRC(&main);

    CYG_PRECONDITION(true, message);
    CYG_PRECONDITIONC(true);

    CYG_POSTCONDITION(true, message);
    CYG_POSTCONDITIONC(true);

    CYG_LOOP_INVARIANT(true, message);
    CYG_LOOP_INVARIANTC(true);

    CYG_INVARIANT(true, message);
    CYG_INVARIANTC(true);
    
    CYG_TEST_PASS_FINISH("true assertions do nothing");
    return 0;
}

