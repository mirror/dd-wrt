//==========================================================================
//
//      tassert1.cxx
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
// Date:                1998-11-27
// Purpose:
// Description:         By default all assertions should be disabled, but
//                      they should compile just fine. This module uses all
//                      assertions except CYGFAIL. As a fringe benefit, all
//                      of the macros get checked for correct argument usage.
//
//####DESCRIPTIONEND####
//==========================================================================


#include <cyg/infra/testcase.h>
#include <cyg/infra/cyg_ass.h>
#include <cstdlib>

#ifdef CYGDBG_USE_ASSERTS
# error Assertions should not be enabled by default.
#endif

static const char message[] = "This should never be seen.";

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
    static void extern_assertions(dummy*);
    bool check_this(cyg_assert_class_zeal) const;
};

int main(int argc, char** argv)
{
    dummy object;
    
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

    CYG_ASSERT_CLASS(  &object, message);
    CYG_ASSERT_CLASSC( &object);
    CYG_ASSERT_CLASSO( object, message);
    CYG_ASSERT_CLASSOC( object);
    CYG_ASSERTCLASS(   &object, message);
    CYG_ASSERTCLASSO(  object, message);

    CYG_PRECONDITION(true, message);
    CYG_PRECONDITION(false, message);
    CYG_PRECONDITIONC(true);
    CYG_PRECONDITIONC(false);
    CYG_PRECONDITION_CLASS(&object, message);
    CYG_PRECONDITION_CLASSC(&object);
    CYG_PRECONDITION_CLASSO(object, message);
    CYG_PRECONDITION_CLASSOC(object);

    CYG_POSTCONDITION(true, message);
    CYG_POSTCONDITION(false, message);
    CYG_POSTCONDITIONC(true);
    CYG_POSTCONDITIONC(false);
    CYG_POSTCONDITION_CLASS(&object, message);
    CYG_POSTCONDITION_CLASSC(&object);
    CYG_POSTCONDITION_CLASSO(object, message);
    CYG_POSTCONDITION_CLASSOC(object);

    CYG_LOOP_INVARIANT(true, message);
    CYG_LOOP_INVARIANT(false, message);
    CYG_LOOP_INVARIANTC(true);
    CYG_LOOP_INVARIANTC(false);
    CYG_LOOP_INVARIANT_CLASS(&object, message);
    CYG_LOOP_INVARIANT_CLASSC(&object);
    CYG_LOOP_INVARIANT_CLASSO(object, message);
    CYG_LOOP_INVARIANT_CLASSOC(object);

    CYG_INVARIANT(true, message);
    CYG_INVARIANT(false, message);
    CYG_INVARIANTC(true);
    CYG_INVARIANTC(false);
    
    dummy::extern_assertions( &object);
    dummy::extern_assertions( 0);
    object.assertions( );
    
    CYG_TEST_PASS_FINISH("disabled assertions do nothing");
    return 0;
}

// A utility routine which performs assertions on this.
void
dummy::assertions()
{
    CYG_INVARIANT_THIS(dummy, message);
    CYG_INVARIANT_THISC(dummy);
    
    CYG_ASSERT_THIS(message);
    CYG_ASSERT_THISC();
    CYG_PRECONDITION_THIS(message);
    CYG_PRECONDITION_THISC();
    CYG_POSTCONDITION_THIS(message);
    CYG_POSTCONDITION_THISC();
    CYG_LOOP_INVARIANT_THIS(message);
    CYG_LOOP_INVARIANT_THISC();
}

// Another utility which gets passed an object pointer.
// This is useful for the ZERO_OR_CLASS() variants.
void
dummy::extern_assertions(dummy* obj)
{
    dummy obj2;
    CYG_INVARIANT_CLASSO(dummy, obj2, message);
    CYG_INVARIANT_CLASSOC(dummy, obj2);
    CYG_INVARIANT_CLASS(dummy, obj, message);
    CYG_INVARIANT_CLASSC(dummy, obj);
    CYG_ASSERT_ZERO_OR_CLASS(obj, message);
    CYG_ASSERT_ZERO_OR_CLASSC(obj);
    CYG_PRECONDITION_ZERO_OR_CLASS(obj, message);
    CYG_PRECONDITION_ZERO_OR_CLASSC(obj);
    CYG_POSTCONDITION_ZERO_OR_CLASS(obj, message);
    CYG_POSTCONDITION_ZERO_OR_CLASSC(obj);
    CYG_LOOP_INVARIANT_ZERO_OR_CLASS(obj, message);
    CYG_LOOP_INVARIANT_ZERO_OR_CLASSC(obj);
}

bool
dummy::check_this(cyg_assert_class_zeal zeal) const
{
    // The default zeal should be cyg_quick.
    switch(zeal) {
    case cyg_quick:
        return true;
        
    case cyg_system_test:
    case cyg_extreme:
    case cyg_thorough:
    case cyg_trivial:
    case cyg_none:
        CYG_TEST_FAIL_FINISH("incorrect default zeal passed to check_this() member function");
        
    default:
        CYG_TEST_FAIL_FINISH("invalid zeal passed to check_this() member function");
    }
    return false;
}

