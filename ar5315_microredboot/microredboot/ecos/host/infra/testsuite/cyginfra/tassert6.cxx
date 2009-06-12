//==========================================================================
//
//      tassert6.cxx
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
// Date:                1998-12-22
// Purpose:
// Description:         This routine checks that all the assertions can
//                      be triggered.
//
//####DESCRIPTIONEND####
//==========================================================================


#define CYG_DECLARE_HOST_ASSERTION_SUPPORT
#define CYGDBG_USE_ASSERTS
#define CYGDBG_INFRA_DEBUG_PRECONDITIONS
#define CYGDBG_INFRA_DEBUG_POSTCONDITIONS
#define CYGDBG_INFRA_DEBUG_LOOP_INVARIANTS
#define CYGDBG_INFRA_DEBUG_INVARIANTS

// Also redefine the zeal
#define CYG_ASSERT_CLASS_ZEAL (cyg_extreme)

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
static int   counter = 0;

static const char message[] = "beware of the leopard";

// A dummy class is needed for some of the assertions.
class dummy {
  private:
    int       random;
  public:
    dummy() {
        random = rand();
    }
    ~dummy() {
        random = 0;
    }
    void assertions();
    void invariant1();
    void invariant2();
    static void invariant3(dummy&);
    static void invariant4(dummy&);
    static void invariant5(dummy*);
    static void invariant6(dummy*);
    static void extern_assertions(dummy*);
    bool check_this(cyg_assert_class_zeal) const;
};

bool
dummy::check_this(cyg_assert_class_zeal zeal) const
{
    // The default zeal should be cyg_quick.
    switch(zeal) {
    case cyg_extreme:
        return false;
    case cyg_system_test:
    case cyg_thorough:
    case cyg_quick:
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
    CYG_TEST_FAIL("CYG_INVARIANT_THIS() did not trigger");
}

void
dummy::invariant2(void)
{
    CYG_INVARIANT_THISC(dummy);
    CYG_TEST_FAIL("CYG_INVARIANT_THISC() did not trigger");
}

void
dummy::invariant3(dummy& obj)
{
    CYG_INVARIANT_CLASSO(dummy, obj, message);
    CYG_TEST_FAIL("CYG_INVARIANT_CLASSO() did not trigger");
}

void
dummy::invariant4(dummy& obj)
{
    CYG_INVARIANT_CLASSOC(dummy, obj);
    CYG_TEST_FAIL("CYG_INVARIANT_CLASSOC() did not trigger");
}

void
dummy::invariant5(dummy* obj)
{
    CYG_INVARIANT_CLASS(dummy, obj, message);
    CYG_TEST_FAIL("CYG_INVARIANT_CLASS() did not trigger");
}

void
dummy::invariant6(dummy* obj)
{
    CYG_INVARIANT_CLASSC(dummy, obj);
    CYG_TEST_FAIL("CYG_INVARIANT_CLASSC() did not trigger");
}

void
dummy::assertions(void)
{
    switch(counter) {

    case 33:
        CYG_ASSERT_THIS(message);
        CYG_TEST_FAIL("CYG_ASSERT_THIS() did not trigger");
        break;

    case 34:
        CYG_ASSERT_THISC();
        CYG_TEST_FAIL("CYG_ASSERT_THISC() did not trigger");
        break;

    case 35:
        CYG_PRECONDITION_THIS(message);
        CYG_TEST_FAIL("CYG_PRECONDITION_THIS() did not trigger");
        break;

    case 36:
        CYG_PRECONDITION_THISC();
        CYG_TEST_FAIL("CYG_PRECONDITION_THISC() did not trigger");
        break;

    case 37:
        CYG_POSTCONDITION_THIS(message);
        CYG_TEST_FAIL("CYG_POSTCONDITION_THIS() did not trigger");
        break;

    case 38:
        CYG_POSTCONDITION_THISC();
        CYG_TEST_FAIL("CYG_POSTCONDITION_THISC() did not trigger");
        break;

    case 39:
        CYG_LOOP_INVARIANT_THIS(message);
        CYG_TEST_FAIL("CYG_LOOP_INVARIANT_THIS() did not trigger");
        break;

    case 40:
        CYG_LOOP_INVARIANT_THISC();
        CYG_TEST_FAIL("CYG_LOOP_INVARIANT_THISC() did not trigger");
        break;

    default:
        CYG_TEST_FAIL("dummy::assertions() invoked for no reason");
        break;
    }
}

void
dummy::extern_assertions(dummy* obj)
{
    switch(counter) {
    case 41:
        CYG_ASSERT_ZERO_OR_CLASS(obj, message);
        CYG_TEST_FAIL("CYG_ASSERT_ZERO_OR_CLASS() did not trigger");
        break;

    case 42:
        CYG_ASSERT_ZERO_OR_CLASSC(obj);
        CYG_TEST_FAIL("CYG_ASSERT_ZERO_OR_CLASSC() did not trigger");
        break;

    case 43:
        CYG_PRECONDITION_ZERO_OR_CLASS(obj, message);
        CYG_TEST_FAIL("CYG_PRECONDITION_ZERO_OR_CLASS() did not trigger");
        break;

    case 44:
        CYG_PRECONDITION_ZERO_OR_CLASSC(obj);
        CYG_TEST_FAIL("CYG_PRECONDITION_ZERO_OR_CLASSC() did not trigger");
        break;

    case 45:
        CYG_POSTCONDITION_ZERO_OR_CLASS(obj, message);
        CYG_TEST_FAIL("CYG_POSTCONDITION_ZERO_OR_CLASS() did not trigger");
        break;

    case 46:
        CYG_POSTCONDITION_ZERO_OR_CLASSC(obj);
        CYG_TEST_FAIL("CYG_POSTCONDITION_ZERO_OR_CLASSC() did not trigger");
        break;

    case 47:
        CYG_LOOP_INVARIANT_ZERO_OR_CLASS(obj, message);
        CYG_TEST_FAIL("CYG_LOOP_INVARIANT_ZERO_OR_CLASS() did not trigger");
        break;

    case 48:
        CYG_LOOP_INVARIANT_ZERO_OR_CLASSC(obj);
        CYG_TEST_FAIL("CYG_LOOP_INVARIANT_ZERO_OR_CLASSC() did not trigger");
        break;

    default:
        CYG_TEST_FAIL("dummy::extern_assertions() invoked for no reason");
        break;
    }
    
}

extern "C"
bool
failure_handler(const char* fn, const char* file, cyg_uint32 line, const char* msg)
{
    failed_assertions++;
    counter++;
    longjmp(setjmp_buffer, 1);
    return true;
}

int
main(int argc, char** argv)
{
    dummy object;

    cyg_assert_install_failure_handler(&failure_handler);
    setjmp(setjmp_buffer);
    
    for ( bool done = false; !done; counter++ ) {
        switch(counter) {
        case 0:
            CYG_ASSERT(false, message);
            CYG_TEST_FAIL("CYG_ASSERT() did not trigger");
            break;

        case 1:
            CYG_ASSERTC(false);
            CYG_TEST_FAIL("CYG_ASSERTC() did not trigger");
            break;

        case 2:
            CYG_FAIL(message);
            CYG_TEST_FAIL("CYG_FAIL() did not trigger");
            break;

        case 3:
            CYG_CHECK_DATA_PTR(0, message);
            CYG_TEST_FAIL("CHECK_CHECK_DATA_PTR() did not trigger");
            break;

        case 4:
            CYG_CHECK_DATA_PTRC(0);
            CYG_TEST_FAIL("CHECK_CHECK_DATA_PTRC() did not trigger");
            break;

        case 5:
            CYG_CHECK_FUNC_PTR(0, message);
            CYG_TEST_FAIL("CHECK_CHECK_FUNC_PTR() did not trigger");
            break;

        case 6:
            CYG_CHECK_DATA_PTRC(0);
            CYG_TEST_FAIL("CHECK_CHECK_FUNC_PTRC() did not trigger");
            break;

        case 7:
            CYG_ASSERT_CLASS(&object, message);
            CYG_TEST_FAIL("CYG_ASSERT_CLASS() did not trigger");
            break;

        case 8:
            CYG_ASSERT_CLASSC(&object);
            CYG_TEST_FAIL("CYG_ASSERT_CLASSC() did not trigger");
            break;

        case 9:
            CYG_ASSERT_CLASSO(object, message);
            CYG_TEST_FAIL("CYG_ASSERT_CLASSO() did not trigger");
            break;

        case 10:
            CYG_ASSERT_CLASSOC(object);
            CYG_TEST_FAIL("CYG_ASSERT_CLASSOC() did not trigger");
            break;

        case 11:
            CYG_PRECONDITION(false, message);
            CYG_TEST_FAIL("CYG_PRECONDITION() did not trigger");
            break;

        case 12:
            CYG_PRECONDITIONC(false);
            CYG_TEST_FAIL("CYG_PRECONDITIONC() did not trigger");
            break;

        case 13:
            CYG_PRECONDITION_CLASS(&object, message);
            CYG_TEST_FAIL("CYG_PRECONDITION_CLASS() did not trigger");
            break;

        case 14:
            CYG_PRECONDITION_CLASSC(&object);
            CYG_TEST_FAIL("CYG_PRECONDITION_CLASSC() did not trigger");
            break;

        case 15:
            CYG_PRECONDITION_CLASSO(object, message);
            CYG_TEST_FAIL("CYG_PRECONDITION_CLASSO() did not trigger");
            break;

        case 16:
            CYG_PRECONDITION_CLASSOC(object);
            CYG_TEST_FAIL("CYG_PRECONDITION_CLASSOC() did not trigger");
            break;
            
        case 17:
            CYG_POSTCONDITION(false, message);
            CYG_TEST_FAIL("CYG_POSTCONDITION() did not trigger");
            break;

        case 18:
            CYG_POSTCONDITIONC(false);
            CYG_TEST_FAIL("CYG_POSTCONDITIONC() did not trigger");
            break;

        case 19:
            CYG_POSTCONDITION_CLASS(&object, message);
            CYG_TEST_FAIL("CYG_POSTCONDITION_CLASS() did not trigger");
            break;

        case 20:
            CYG_POSTCONDITION_CLASSC(&object);
            CYG_TEST_FAIL("CYG_POSTCONDITION_CLASSC() did not trigger");
            break;

        case 21:
            CYG_POSTCONDITION_CLASSO(object, message);
            CYG_TEST_FAIL("CYG_POSTCONDITION_CLASSO() did not trigger");
            break;

        case 22:
            CYG_POSTCONDITION_CLASSOC(object);
            CYG_TEST_FAIL("CYG_POSTCONDITION_CLASSOC() did not trigger");
            break;

        case 23:
            CYG_LOOP_INVARIANT(false, message);
            CYG_TEST_FAIL("CYG_LOOP_INVARIANT() did not trigger");
            break;

        case 24:
            CYG_LOOP_INVARIANTC(false);
            CYG_TEST_FAIL("CYG_LOOP_INVARIANTC() did not trigger");
            break;

        case 25:
            CYG_LOOP_INVARIANT_CLASS(&object, message);
            CYG_TEST_FAIL("CYG_LOOP_INVARIANT_CLASS() did not trigger");
            break;

        case 26:
            CYG_LOOP_INVARIANT_CLASSC(&object);
            CYG_TEST_FAIL("CYG_LOOP_INVARIANT_CLASSC() did not trigger");
            break;

        case 27:
            CYG_LOOP_INVARIANT_CLASSO(object, message);
            CYG_TEST_FAIL("CYG_LOOP_INVARIANT_CLASSO() did not trigger");
            break;

        case 28:
            CYG_LOOP_INVARIANT_CLASSOC(object);
            CYG_TEST_FAIL("CYG_LOOP_INVARIANT_CLASSOC() did not trigger");
            break;

        case 29:
            CYG_INVARIANT(false, message);
            CYG_TEST_FAIL("CYG_INVARIANT() did not trigger");
            break;

        case 30:
            CYG_INVARIANTC(false);
            CYG_TEST_FAIL("CYG_INVARIANTC() did not trigger");
            break;

        case 31:
            object.invariant1();
            break;
            
        case 32:
            object.invariant2();
            break;
            
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
            object.assertions();
            break;

        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
            dummy::extern_assertions(&object);
            break;

        case 49:
            dummy::invariant3(object);
            break;
            
        case 50:
            dummy::invariant4(object);
            break;
            
        case 51:
            dummy::invariant5(&object);
            break;
            
        case 52:
            dummy::invariant6(&object);
            break;
            
        default:
            done = true;
            counter--;  // About to get incremented again...
            break;
        }
    }

    if (failed_assertions != 53) {
        CYG_TEST_FAIL("Broken test case, not all assertions have been tried");
    }
    
    if (failed_assertions == counter) {
        CYG_TEST_PASS("All assertions trigger successfully");
    } else {
        CYG_TEST_FAIL("Not all assertions trigger");
    }
    
    return 0;
}

