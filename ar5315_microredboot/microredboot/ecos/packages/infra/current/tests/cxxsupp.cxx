//==========================================================================
//
//        cxxsupp.cxx
//
//        C++ runtime support test
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Nick Garnett <nickg@calivar.com>
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting the copyright
// holders.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     nickg
// Contributors:  nickg
// Date:          2003-04-01
// Description:   Simple test for C++ runtime support.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/isoinfra.h>

#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>

// The H8300 does not have C++ support in its toolchain
#ifndef CYGPKG_HAL_H8300

#include <new>

//==========================================================================

class Pure
{
protected:    
    int instance;
public:
    Pure(int i);
    virtual void pure_fun1(void) = 0;
    virtual void pure_fun2(void) = 0;
    virtual void impure_fun1(void);
    inline void inline_fun1(void);
};

Pure::Pure(int i)
{
    instance = i;
    diag_printf("%s(%d) called\n",__PRETTY_FUNCTION__,instance);    
}

void Pure::impure_fun1()
{
    diag_printf("%s(%d) called\n",__PRETTY_FUNCTION__,instance);
}

inline void Pure::inline_fun1()
{
    diag_printf("%s(%d) called\n",__PRETTY_FUNCTION__,instance);
}

//==========================================================================

class Derived : public Pure
{
public:
    Derived(int i);
    void pure_fun1(void);
    void pure_fun2(void);
    void impure_fun2(void);
};

Derived::Derived(int i)
    : Pure(i)
{
    diag_printf("%s(%d) called\n",__PRETTY_FUNCTION__,instance);
}

void Derived::pure_fun1(void)
{
    diag_printf("%s(%d) called\n",__PRETTY_FUNCTION__,instance);
}

void Derived::pure_fun2(void)
{
    diag_printf("%s(%d) called\n",__PRETTY_FUNCTION__,instance);
}


void Derived::impure_fun2(void)
{
    diag_printf("%s(%d) called\n",__PRETTY_FUNCTION__,instance);
}

//==========================================================================

__externC void
cyg_start( void )
{

    CYG_TEST_INIT();
    
    Derived derived(1);
    Pure *pure = &derived;

    CYG_TEST_INFO("Calling derived members");
    derived.pure_fun1();
    derived.pure_fun2();
    derived.impure_fun1();
    derived.impure_fun2();
    derived.inline_fun1();

    CYG_TEST_INFO("Calling pure members");
    pure->pure_fun1();
    pure->pure_fun2();
    pure->impure_fun1();
    pure->inline_fun1();

#if CYGINT_ISO_MALLOC
    Derived *derived2 = new Derived(2);
    Pure *pure2 = derived2;
    
    CYG_TEST_INFO("Calling derived2 members");
    derived2->pure_fun1();
    derived2->pure_fun2();
    derived2->impure_fun1();
    derived2->impure_fun2();
    derived2->inline_fun1();

    CYG_TEST_INFO("Calling pure2 members");
    pure2->pure_fun1();
    pure2->pure_fun2();
    pure2->impure_fun1();
    pure2->inline_fun1();

    delete derived2;
    
#else
    CYG_TEST_INFO("No malloc support, new and delete not tested");
#endif
    
    CYG_TEST_PASS_FINISH("C++ Support OK");
}

//==========================================================================

#else

__externC void
cyg_start( void )
{

    CYG_TEST_INIT();

    CYG_TEST_NA("C++ not supported on this architecture\n");
}

#endif

//==========================================================================
// EOF cxxsupp.cxx
