//==========================================================================
//
//      cdl2.cxx
//
//      Tests for the CdlHandle class.
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
// Date:                1999-01-12
// Description:         Large parts of libcdl are implemented using a
//                      CdlHandle template and a CdlRefcountSupport class.
//                      This tests check that these both work as expected.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cstdio>
#include <cdlconfig.h>
#include <cdl.hxx>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/testcase.h>
#include <cstdlib>

#ifndef CYGBLD_LIBCDL_USE_SMART_POINTERS
int
main(int argc, char** argv)
{
    CYG_TEST_FAIL_FINISH("Smart pointers not yet enabled - waiting for a working version of Visual C++");
    return EXIT_FAILURE;
}
#else // CYGBLD_LIBCDL_USE_SMART_POINTERS
// ----------------------------------------------------------------------------
// Miscellaneous statics.

// This routine controls the return value of a class1_body check_this()
// operation, allowing test code to make sure that using check_this()
// on a smart pointer works as expected.
static bool check_this_ok = true;

// ----------------------------------------------------------------------------
// This test case makes use of three implementation classes. It is necessary
// to have forward declarations of these, and then it is possible to define
// handle classes for each one.
class class01_body;
class class02_body;
class derived_body;

typedef CdlHandle<class01_body> class01;
typedef CdlHandle<class02_body> class02;
typedef CdlHandle<derived_body> derived;

// ----------------------------------------------------------------------------
// This test needs three additional classes which are reference-counted and
// which are accessed via CdlHandle smart pointers. It is necessary to start 

class class01_body : public CdlRefcountSupport {

    friend class CdlTest;
    
  public:

    static int class01_objects;
    class01_body() : CdlRefcountSupport() {
        class01_objects++;
        object_number = class01_objects;
        modifiable    = 0;
        class01_body_cookie = class01_body_magic;
    }
    ~class01_body() {
        class01_objects--;
        class01_body_cookie = class01_body_invalid;
    }
    int
    get_number(void) {
        return object_number;
    }
    void
    modify(void) {
        modifiable++;
    }
    bool check_this(cyg_assert_class_zeal zeal) const {
        CYG_UNUSED_PARAM(cyg_assert_class_zeal, zeal);
        if (class01_body_magic != class01_body_cookie) {
            return false;
        }
        return check_this_ok;
    }
    
  private:
    // Which object is this?
    int object_number;
    int modifiable;
    
    class01_body(const class01_body&);
    class01_body& operator=(const class01_body&);
    enum {
        class01_body_invalid     = 0,
        class01_body_magic       = 0x015b19d6
    } class01_body_cookie;
};

class class02_body : public CdlRefcountSupport {

    friend class CdlTest;
    
  public:
    static int class02_objects;
    class02_body() : CdlRefcountSupport() {
        class02_objects++;
        class02_body_cookie = class02_body_magic;
    }
    ~class02_body() {
        class02_objects--;
        class02_body_cookie = class02_body_invalid;
    }

    bool check_this(cyg_assert_class_zeal zeal) const {
        CYG_UNUSED_PARAM(cyg_assert_class_zeal, zeal);
        return class02_body_magic == class02_body_cookie;
    }

  private:
    class02_body(const class02_body&);
    class02_body& operator=(const class02_body&);
    enum {
        class02_body_invalid     = 0,
        class02_body_magic       = 0x3225c96c
    } class02_body_cookie;
};

class derived_body : public class01_body {

    friend class CdlTest;
    
  public:
    static int derived_objects;
    derived_body() : class01_body() {
        derived_objects++;
        derived_body_cookie = derived_body_magic;
    }
    ~derived_body() {
        derived_objects--;
        derived_body_cookie = derived_body_invalid;
    }
    bool check_this(cyg_assert_class_zeal zeal) const {
        if (derived_body_magic != derived_body_cookie) {
            return false;
        }
        return class01_body::check_this(zeal);
    }

  private:
    derived_body(const derived_body&);
    derived_body& operator=(const derived_body&);
    enum {
        derived_body_invalid    = 0,
        derived_body_magic      = 0x7ed15350
    } derived_body_cookie;
};

int class01_body::class01_objects   = 0;
int class02_body::class02_objects   = 0;
int derived_body::derived_objects   = 0;

// ----------------------------------------------------------------------------
// The actual test code.

bool
check_const_arg(const class01 const_ptr)
{
    // Make sure that read-only access is allowed and goes to the right
    // object
    if (!const_ptr->check_this(cyg_quick)) {
        CYG_TEST_FAIL("check_this() on a constant pointer should be fine");
        return false;
    }
    check_this_ok = false;
    if (const_ptr->check_this(cyg_quick)) {
        CYG_TEST_FAIL("check_this() on a constant pointer should be fine");
        check_this_ok = true;
        return false;
    }
    check_this_ok = true;
    return true;
}

int
main(int argc, char** argv)
{
    bool ok = true;

    // Make sure that smart pointers do not impose any kind of overhead.
    if ((sizeof(void *) != sizeof(class01)) ||
        (sizeof(void *) != sizeof(class02)) ||
        (sizeof(void *) != sizeof(derived))) {

        CYG_TEST_FAIL("smart pointers are not the same size as dumb pointers");
    } else {
        CYG_TEST_PASS("smart pointers are the same size as dumb pointers");
    }
    
    // Start by creating a number of objects to be manipulated.
    class01_body *      class01_obj1    = new class01_body;
    class01_body *      class01_obj2    = new class01_body;
    class02_body *      class02_obj1    = new class02_body;
    derived_body *      derived_obj1    = new derived_body;

    // Quick sanity check
    if ((1 != derived_body::derived_objects) ||
        (1 != class02_body::class02_objects) ||
        (3 != class01_body::class01_objects)) {
        CYG_TEST_FAIL("Testcase has created an invalid number of objects");
    }

    // Convert the basic objects to smart pointers. If this code compiles
    // then the test succeeds.
    class01     class01_ptr1    = class01(class01_obj1);
    class01     class01_ptr2    = class01(class01_obj2);
    class02     class02_ptr1    = class02(class02_obj1);
    derived     derived_ptr1    = derived(derived_obj1);
    CYG_TEST_PASS("conversion to smart pointers works");

    // Also create a couple of other smart pointers. These should be
    // initialised to 0.
    class01      class01_ptr3;
    class01      class01_ptr4     = 0;
    class01      class01_ptr5     = class01(0);
    CYG_TEST_PASS("smart pointers can have the value zero");

    // Try to dereference the smart pointers.
    if ((1 != class01_ptr1->get_number()) ||
        (2 != class01_ptr2->get_number()) ||
        (3 != derived_ptr1->get_number())) {
        CYG_TEST_FAIL("-> dereferencing operator broken");
    } else {
        CYG_TEST_PASS("-> dereferencing operator functional");
    }
    if ((1 != (*class01_ptr1).get_number()) ||
        (2 != (*class01_ptr2).get_number()) ||
        (3 != (*derived_ptr1).get_number())) {
        CYG_TEST_FAIL("* dereferencing operator broken");
    } else {
        CYG_TEST_PASS("* dereferencing operator functional");
    }

    // Also try to access the check_this() member functions
    if (!class01_ptr1->check_this(cyg_quick)) {
    }
    
    // Do a couple of if's. This checks that the !operator is
    // functional. Some of the checks are there to make sure that the
    // compiler does the right thing.
    ok = true;
    if (!class01_ptr1) {
        CYG_TEST_FAIL("!(assigned smart pointer) is true");
        ok = false;
    }
    if (0 == class01_ptr1) {
        CYG_TEST_FAIL("0 == assigned smart pointer");
        ok = false;
    }
    if (0 != class01_ptr3) {
        CYG_TEST_FAIL("0 != unassigned smart pointer");
        ok = false;
    }
    if (class01_ptr1 == 0) {
        CYG_TEST_FAIL("0 == assigned smart pointer");
        ok = false;
    }
    if (class01_ptr3 != 0) {
        CYG_TEST_FAIL("0 != unassigned smart pointer");
        ok = false;
    }
    if (class01_ptr1 == class01_ptr2) {
        CYG_TEST_FAIL("comparing two different smart pointers succeeds");
        ok = false;
    }
    if (class01_ptr1 != class01_ptr2) {
        // Do nothing
    } else {
        CYG_TEST_FAIL("comparing two different smart pointers succeeds");
        ok = false;
    }
#if 0
    // Comparing base and derived smart pointers directly does not work yet.
    if (class01_ptr1 == derived_ptr1) {
        CYG_TEST_FAIL("comparing different base and derived pointers succeeds");
    }
#endif
    if (ok) {
        CYG_TEST_PASS("smart pointer comparisons work");
    }
    
    // Try some  assignment operators.
    class01_ptr3 = class01_ptr1;
    class01_ptr4 = derived_ptr1;
    class01_ptr5 = class01_ptr2;

    // After doing all of these assignments there should be no change in
    // the number of underlying objects.
    ok = true;
    if ((1 != derived_body::derived_objects) ||
        (1 != class02_body::class02_objects)   ||
        (3 != class01_body::class01_objects)) {
        ok = false;
        CYG_TEST_FAIL("Assignment of smart pointers has changed the underlying number of objects");
    }
    if ((class01_ptr1 != class01_ptr3) ||
        (class01_ptr2 != class01_ptr5)) {
        ok = false;
        CYG_TEST_FAIL("Assignment of smart pointers has not worked");
    }
    if (class01_ptr4.get_dumb_pointer() != derived_ptr1.get_dumb_pointer()) {
        ok = false;
        CYG_TEST_FAIL("Assignment of derived to base smart pointer has not worked");
    }
    if ((2 != class01_ptr1->get_refcount()) ||
        (2 != class01_ptr2->get_refcount()) ||
        (2 != class01_ptr4->get_refcount()) ||
        (2 != derived_ptr1->get_refcount())) {
        ok = false;
        CYG_TEST_FAIL("Reference counts after assignment operators do not match up");
    }
    if (ok) {
        CYG_TEST_PASS("Assignment of smart pointers");
    }

    // Now try assigning zero. Incidentally this is necessary if the underlying
    // objects are to be destroyed again at the end.
    class01_ptr3 = 0;
    class01_ptr4 = 0;
    class01_ptr5 = 0;

    ok = true;
    if (0 != class01_ptr3) {
        ok = false;
        CYG_TEST_FAIL("assigning 0 to a smart pointer does not work");
    }
    if ((1 != class01_ptr1->get_refcount()) ||
        (1 != class01_ptr2->get_refcount()) ||
        (1 != derived_ptr1->get_refcount())) {
        ok = false;
        CYG_TEST_FAIL("Reference counts after assignment operators do not match up");
    }
    if (ok) {
        CYG_TEST_PASS("Assigning zero to smart pointers");
    }

    // Make sure that implicit casts to const work. This is really
    // a compiler test.
    if (check_const_arg(class01_ptr1) &&
        check_const_arg(derived_ptr1)) {
        CYG_TEST_PASS("Implicit cast to const smart pointer");
    }
    
#if 0
    // All of this code should fail to compile.
    // Applying delete to a smart pointer does not work. Use destroy() instead.
    delete class01_ptr1;
#endif
#if 0
    // Attempts to do incompatible assignments should fail.
    class01_ptr1 = class02_ptr1;
#endif
#if 0    
    // Comparing completely different types should fail.
    if (class01_ptr1 == class02_ptr1) {
        CYG_TEST_FAIL("it should not be possible to compare objects of different types");
    }
#endif
#if 0
    {
        const class01 const_class01_ptr = class01_ptr1;
        const_class01_ptr->modify();
    }
#endif
#if 0
    {
        const class01 const_derived_ptr = derived_ptr1;
        const_derived_ptr->modify();
    }
#endif
    
    // Check that destroy() actually gets rid of the underlying objects.
    class01_ptr1.destroy();
    class01_ptr2.destroy();
    class02_ptr1.destroy();
    derived_ptr1.destroy();
    if ((0 != derived_body::derived_objects) ||
        (0 != class02_body::class02_objects)    ||
        (0 != class01_body::class01_objects)) {
        CYG_TEST_FAIL("There are still objects after the smart pointers have been destroyed");
    } else {
        CYG_TEST_PASS("Using destroy() on the smart pointers cleans up the underlying objects");
    }

    return EXIT_SUCCESS;
}
#endif
