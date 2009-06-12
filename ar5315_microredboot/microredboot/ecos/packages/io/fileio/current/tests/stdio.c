//==========================================================================
//
//      stdio.c
//
//      Test stdio integration of fileio system
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
// Author(s):           nickg
// Contributors:        nickg
// Date:                2000-05-25
// Purpose:             Test stdio integration of fileio system
// Description:         This test uses the testfs to check that the options
//                      in the STDIO package that enable use of the fileio
//                      API work correctly. 
//                      
//                      
//                      
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <stdio.h>
#include <string.h>

#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>            // HAL polled output

//==========================================================================
// Include the test filesystem.
// If we could make tests out of multiple files, then we could just link
// against the object file for this rather than including it.

#include "testfs.c"

//==========================================================================

#define SHOW_RESULT( _fn, _res ) \
diag_printf("<INFO>: " #_fn "() returned %d %s\n", _res, _res<0?strerror(errno):"");


#ifdef CYGPKG_LIBC_STDIO

//==========================================================================
// main

int main( int argc, char **argv )
{
    int err;
    FILE *f;
    fpos_t fpos;
    
    int flibble = 4567;
    char *wibble = "abcdefghijk";

    int flibble1;
    char wibble1[20];
    
    CYG_TEST_INIT();

    
    f = fopen("/foo", "w" );
    if( f == NULL ) SHOW_RESULT( fopen, -1 );

    err = fprintf(f, "flibble %d wibble %s\n", flibble, wibble );
    if( err < 0 ) SHOW_RESULT( fprintf, err );
    
    err = fprintf(f, "another flibble %d another wibble %s\n", flibble, wibble );
    if( err < 0 ) SHOW_RESULT( fprintf, err );
    
    err = fclose( f );
    if( err == EOF ) SHOW_RESULT( fclose, -1 );


    
    f = fopen("/foo", "r" );
    if( f == NULL ) SHOW_RESULT( fopen, -1 );

    err = fscanf(f, "flibble %d wibble %s\n", &flibble1, wibble1 );
    if( err < 0 ) SHOW_RESULT( fscanf, err );

    diag_printf("<INFO>: flibble1 %d wibble1 %s\n",flibble1,wibble1);
    
    CYG_TEST_CHECK( flibble1 == flibble , "Bad flibble result from fscanf");
    CYG_TEST_CHECK( strcmp(wibble,wibble1) == 0, "Bad wibble result from fscanf");


    err = fgetpos( f, &fpos );
    if( err < 0 ) SHOW_RESULT( fgetpos, err );    
    
    err = fseek( f, 0, SEEK_SET );
    if( err < 0 ) SHOW_RESULT( fseek, err );

    err = fscanf(f, "flibble %d wibble %s\n", &flibble1, wibble1 );
    if( err < 0 ) SHOW_RESULT( fscanf, err );

    diag_printf("<INFO>: flibble1 %d wibble1 %s\n",flibble1,wibble1);
    
    CYG_TEST_CHECK( flibble1 == flibble , "Bad flibble result from fscanf");
    CYG_TEST_CHECK( strcmp(wibble,wibble1) == 0, "Bad wibble result from fscanf");


    err = fseek( f, 0, SEEK_END );
    if( err < 0 ) SHOW_RESULT( fseek, err );

    err = fsetpos( f, &fpos );
    if( err < 0 ) SHOW_RESULT( fsetpos, err );    
    
    err = fscanf(f, "another flibble %d another wibble %s\n", &flibble1, wibble1 );
    if( err < 0 ) SHOW_RESULT( fscanf, err );

    diag_printf("<INFO>: flibble1 %d wibble1 %s\n",flibble1,wibble1);
    
    CYG_TEST_CHECK( flibble1 == flibble , "Bad flibble result from fscanf");
    CYG_TEST_CHECK( strcmp(wibble,wibble1) == 0, "Bad wibble result from fscanf");


    
    err = fclose( f );


    
    CYG_TEST_PASS_FINISH("stdio");
    
    return 0;
}

#else

// -------------------------------------------------------------------------

int main( int argc, char **argv )
{

    CYG_TEST_INIT();

    CYG_TEST_NA("stdio");
}

#endif    

// -------------------------------------------------------------------------
// EOF stdio.c
