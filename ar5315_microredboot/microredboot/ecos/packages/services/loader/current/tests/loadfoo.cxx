//==========================================================================
//
//        loadfoo.cxx
//
//        Dynamic library test program
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     nickg
// Contributors:  nickg
// Date:          2000-11-20
// Description:   Dynamic library test
//
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/kernel.h>
#include <pkgconf/hal.h>

#include <cyg/infra/testcase.h>

//#include <cyg/loader/loader.hxx>

#include <dlfcn.h>

//==========================================================================
// Include a version of the library ELF file converted to a C table

#include "tests/libfoo.so.c"

//==========================================================================

externC void cyg_dl_force(void);

typedef void vfn();

int fee_data = 0;

//int main( int argc, char **argv )
externC void cyg_start()
{
    CYG_TEST_INIT();

    CYG_TEST_INFO( "LoadFoo: started" );
#if 0    
    Cyg_LoaderStream_Mem memstream(libfoo, sizeof( libfoo ) );

    Cyg_LoadObject *obj;

//    cyg_dl_force();
    
    Cyg_Loader::loader->load( memstream, 0, &obj );

    vfn *foo = (vfn *)obj->symbol( "foo" );

#else

    void *fooh = dlopenmem( libfoo, sizeof(libfoo), RTLD_NOW );

    vfn *foo = (vfn *)dlsym( fooh, "foo" );
    
#endif

    
    if( foo )
    {
        CYG_TEST_INFO( "LoadFoo: foo() call" );    
        foo();
        CYG_TEST_INFO( "LoadFoo: foo() returned" );            
    }
    else
    {
        CYG_TEST_FAIL_FINISH( "LoadFoo: foo() NULL!!!" );    
    }

    CYG_TEST_PASS_FINISH("LoadFoo: OK");
    
}

//==========================================================================

externC void fee(int x)
{
    CYG_TEST_INFO( "LoadFoo: fee() called" );    
    fee_data = x;
    CYG_TEST_INFO( "LoadFoo: fee() returning" );    
}

//==========================================================================
// EOF loadfoo.cxx
