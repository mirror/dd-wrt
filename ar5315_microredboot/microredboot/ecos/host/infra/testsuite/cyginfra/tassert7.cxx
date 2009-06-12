//==========================================================================
//
//      tassert7.cxx
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
// Description:         This routine checks the invariant assertions.
//                      The entry tests will have been taken care of by
//                      tassert6.cxx, but it is also necessary to check
//                      the exit tests.
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
#include <cstdlib>
#include <csetjmp>
#include <cstring>

// This is used to "recover" from an assertion failure
static jmp_buf setjmp_buffer;

// The number of assertions that have triggered.
static int failed_assertions = 0;

// The number of assertions that have been triggered.
static int counter = 0;

// Are objects currently valid?
static bool check_this_should_fail = false;

static const char message[] = "so long and thanks for all the fish";

class dummy {
  private:
    int         random;

  public:
    dummy() {
        random = rand();
    }
    ~dummy() {
        random = 0;
    }

    void invariant1();
    void invariant2();
    static void invariant3(dummy&);
    static void invariant4(dummy&);
    static void invariant5(dummy*);
    static void invariant6(dummy*);
    bool check_this(cyg_assert_class_zeal) const;
};

bool
dummy::check_this(cyg_assert_class_zeal zeal) const
{
    // The default zeal should be cyg_quick.
    switch(zeal) {
    case cyg_quick:
        return !check_this_should_fail;
        
    case cyg_system_test:
    case cyg_extreme:
    case cyg_thorough:
    case cyg_trivial:
    case cyg_none:
        CYG_TEST_FAIL_FINISH("incorrect default zeal passed to check_this() member function");
        break;
    default:
        CYG_TEST_FAIL_FINISH("invalid zeal passed to check_this() member function");
        break;
    }
    return false;
}

void
dummy::invariant1(void)
{
    CYG_INVARIANT_THIS(dummy, message);
    check_this_should_fail = true;
}

void
dummy::invariant2(void)
{
    CYG_INVARIANT_THISC(dummy);
    check_this_should_fail = true;
}

void
dummy::invariant3(dummy& obj)
{
    CYG_INVARIANT_CLASSO(dummy, obj, message);
    check_this_should_fail = true;
}

void
dummy::invariant4(dummy& obj)
{
    CYG_INVARIANT_CLASSOC(dummy, obj);
    check_this_should_fail = true;
}

void
dummy::invariant5(dummy* obj)
{
    CYG_INVARIANT_CLASS(dummy, obj, message);
    check_this_should_fail = true;
}

void
dummy::invariant6(dummy* obj)
{
    CYG_INVARIANT_CLASSC(dummy, obj);
    check_this_should_fail = true;
}

extern "C"
bool
failure_handler(const char* fn, const char* file, cyg_uint32 line, const char* msg)
{
    if (false == check_this_should_fail) {
        CYG_TEST_FAIL("assertion triggered when everything should be ok");
    }
    failed_assertions++;
    counter++;
    longjmp(setjmp_buffer, 1);
    return true;
}

int
main(int argc, char **argv)
{
    dummy object;

    cyg_assert_install_failure_handler(&failure_handler);
    setjmp(setjmp_buffer);
    
    for ( bool done = false; !done; counter++ ) {
        check_this_should_fail = false;
        
        switch(counter) {
        case 0:
            object.invariant1();
            CYG_TEST_FAIL("CYG_INVARIANT_THIS() test should not have returned");
            break;
            
        case 1:
            object.invariant2();
            CYG_TEST_FAIL("CYG_INVARIANT_THISC() test should not have returned");
            break;
            
        case 2:
            dummy::invariant3(object);
            CYG_TEST_FAIL("CYG_INVARIANT_CLASSO() test should not have returned");
            break;
            
        case 3:
            dummy::invariant4(object);
            CYG_TEST_FAIL("CYG_INVARIANT_CLASSOC() test should not have returned");
            break;
            
        case 4:
            dummy::invariant5(&object);
            CYG_TEST_FAIL("CYG_INVARIANT_CLASS() test should not have returned");
            break;
            
        case 5:
            dummy::invariant6(&object);
            CYG_TEST_FAIL("CYG_INVARIANT_CLASSC() test should not have returned");
            break;

        default:
            done = true;
            counter--;  // About to get incremented again...
            break;
        }
    }

    if (failed_assertions != 6) {
        CYG_TEST_FAIL("Broken test case, not all assertions have been tried");
    }
    
    if (failed_assertions == counter) {
        CYG_TEST_PASS("All assertions trigger successfully");
    } else {
        CYG_TEST_FAIL("Not all assertions trigger");
    }
    return 0;
}
