/* ==========================================================================
 *
 *      tassert4.c
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
 * Description:         A C equivalent of tassert1.cxx
 *
 *####DESCRIPTIONEND####
 *==========================================================================
 */

#include <cyg/infra/testcase.h>
#include <cyg/infra/cyg_ass.h>
#include <stdlib.h>

#ifdef CYGDBG_USE_ASSERTS
# error Assertions should not be enabled by default.
#endif

static const char message[] = "This should never be seen.";

int main(int argc, char** argv)
{
    CYG_ASSERT( true, message);
    CYG_ASSERT( false, message);
    CYG_ASSERTC(true);
    CYG_ASSERTC(false);
    
    CYG_FAIL(message);
    
    CYG_CHECK_DATA_PTR( &argc, message);
    CYG_CHECK_DATA_PTR( 0,     message);
    CYG_CHECK_FUNC_PTR( &main, message);
    CYG_CHECK_FUNC_PTR( 0,     message);
    CYG_CHECK_DATA_PTRC(&argc);
    CYG_CHECK_DATA_PTRC(0);
    CYG_CHECK_FUNC_PTRC(&main);
    CYG_CHECK_FUNC_PTRC(0);

    CYG_PRECONDITION(true, message);
    CYG_PRECONDITION(false, message);
    CYG_PRECONDITIONC(true);
    CYG_PRECONDITIONC(false);

    CYG_POSTCONDITION(true, message);
    CYG_POSTCONDITION(false, message);
    CYG_POSTCONDITIONC(true);
    CYG_POSTCONDITIONC(false);

    CYG_LOOP_INVARIANT(true, message);
    CYG_LOOP_INVARIANT(false, message);
    CYG_LOOP_INVARIANTC(true);
    CYG_LOOP_INVARIANTC(false);

    CYG_INVARIANT(true, message);
    CYG_INVARIANT(false, message);
    CYG_INVARIANTC(true);
    CYG_INVARIANTC(false);
    
    CYG_TEST_PASS_FINISH("disabled assertions in C code do nothing");
    return 0;
}

#ifdef CYG_ASSERT_CLASS
# error CYG_ASSERT_CLASS macro should not be defined in C code
#endif
#ifdef CYG_ASSERT_CLASSC
# error CYG_ASSERT_CLASSC macro should not be defined in C code
#endif
#ifdef CYG_ASSERT_CLASSO
# error CYG_ASSERT_CLASSO macro should not be defined in C code
#endif
#ifdef CYG_ASSERT_CLASSOC
# error CYG_ASSERT_CLASSOC macro should not be defined in C code
#endif
#ifdef CYG_ASSERT_ZERO_OR_CLASS
# error CYG_ASSERT_ZERO_OR_CLASS macro should not be defined in C code
#endif
#ifdef CYG_ASSERT_ZERO_OR_CLASSC
# error CYG_ASSERT_ZERO_OR_CLASSC macro should not be defined in C code
#endif
#ifdef CYG_ASSERT_THIS
# error CYG_ASSERT_THIS macro should not be defined in C code
#endif
#ifdef CYG_ASSERT_THISC
# error CYG_ASSERT_THISC macro should not be defined in C code
#endif
#ifdef CYGDBG_DEFINE_CHECK_THIS
# error CYGDBG_DEFINE_CHECK_THIS macro should not be defined in C code
#endif

#ifdef CYG_PRECONDITION_CLASS
# error CYG_PRECONDITION_CLASS macro should not be defined in C code
#endif
#ifdef CYG_PRECONDITION_CLASSC
# error CYG_PRECONDITION_CLASSC macro should not be defined in C code
#endif
#ifdef CYG_PRECONDITION_CLASSO
# error CYG_PRECONDITION_CLASSO macro should not be defined in C code
#endif
#ifdef CYG_PRECONDITION_CLASSOC
# error CYG_PRECONDITION_CLASSOC macro should not be defined in C code
#endif
#ifdef CYG_PRECONDITION_ZERO_OR_CLASS
# error CYG_PRECONDITION_ZERO_OR_CLASS macro should not be defined in C code
#endif
#ifdef CYG_PRECONDITION_ZERO_OR_CLASSC
# error CYG_PRECONDITION_ZERO_OR_CLASSC macro should not be defined in C code
#endif
#ifdef CYG_PRECONDITION_THIS
# error CYG_PRECONDITION_THIS macro should not be defined in C code
#endif
#ifdef CYG_PRECONDITION_THISC
# error CYG_PRECONDITION_THISC macro should not be defined in C code
#endif

#ifdef CYG_POSTCONDITION_CLASS
# error CYG_POSTCONDITION_CLASS macro should not be defined in C code
#endif
#ifdef CYG_POSTCONDITION_CLASSC
# error CYG_POSTCONDITION_CLASSC macro should not be defined in C code
#endif
#ifdef CYG_POSTCONDITION_CLASSO
# error CYG_POSTCONDITION_CLASSO macro should not be defined in C code
#endif
#ifdef CYG_POSTCONDITION_CLASSOC
# error CYG_POSTCONDITION_CLASSOC macro should not be defined in C code
#endif
#ifdef CYG_POSTCONDITION_ZERO_OR_CLASS
# error CYG_POSTCONDITION_ZERO_OR_CLASS macro should not be defined in C code
#endif
#ifdef CYG_POSTCONDITION_ZERO_OR_CLASSC
# error CYG_POSTCONDITION_ZERO_OR_CLASSC macro should not be defined in C code
#endif
#ifdef CYG_POSTCONDITION_THIS
# error CYG_POSTCONDITION_THIS macro should not be defined in C code
#endif
#ifdef CYG_POSTCONDITION_THISC
# error CYG_POSTCONDITION_THISC macro should not be defined in C code
#endif

#ifdef CYG_LOOP_INVARIANT_CLASS
# error CYG_LOOP_INVARIANT_CLASS macro should not be defined in C code
#endif
#ifdef CYG_LOOP_INVARIANT_CLASSC
# error CYG_LOOP_INVARIANT_CLASSC macro should not be defined in C code
#endif
#ifdef CYG_LOOP_INVARIANT_CLASSO
# error CYG_LOOP_INVARIANT_CLASSO macro should not be defined in C code
#endif
#ifdef CYG_LOOP_INVARIANT_CLASSOC
# error CYG_LOOP_INVARIANT_CLASSOC macro should not be defined in C code
#endif
#ifdef CYG_LOOP_INVARIANT_ZERO_OR_CLASS
# error CYG_LOOP_INVARIANT_ZERO_OR_CLASS macro should not be defined in C code
#endif
#ifdef CYG_LOOP_INVARIANT_ZERO_OR_CLASSC
# error CYG_LOOP_INVARIANT_ZERO_OR_CLASSC macro should not be defined in C code
#endif
#ifdef CYG_LOOP_INVARIANT_THIS
# error CYG_LOOP_INVARIANT_THIS macro should not be defined in C code
#endif
#ifdef CYG_LOOP_INVARIANT_THISC
# error CYG_LOOP_INVARIANT_THISC macro should not be defined in C code
#endif

#ifdef CYG_INVARIANT_CLASS
# error CYG_INVARIANT_CLASS macro should not be defined in C code
#endif
#ifdef CYG_INVARIANT_CLASSC
# error CYG_INVARIANT_CLASSC macro should not be defined in C code
#endif
#ifdef CYG_INVARIANT_CLASSO
# error CYG_INVARIANT_CLASSO macro should not be defined in C code
#endif
#ifdef CYG_INVARIANT_CLASSOC
# error CYG_INVARIANT_CLASSOC macro should not be defined in C code
#endif
#ifdef CYG_INVARIANT_THIS
# error CYG_INVARIANT_THIS macro should not be defined in C code
#endif
#ifdef CYG_INVARIANT_THISC
# error CYG_INVARIANT_THISC macro should not be defined in C code
#endif



