//==========================================================================
//
//      entable.c
//
//      Convert binary file to C table
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
// Date:                2000-11-25
// Purpose:             Convert binary file to C table
// Description:         This is a simple host-side program that converts a binary
//                      file on it's input to a C format table of unsigned chars
//                      on its output. The single argument gives the name that the
//                      table should be given.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <stdio.h>

int main( int argc, char **argv )
{
    int i;

    if( argc != 2 )
    {
        fprintf( stderr, "usage: %s <table name>\n",argv[0]);
        exit(1);
    }
    
    printf( "unsigned char %s[] = {\n", argv[1]);

    for(i = 0; ; i++)
    {
        int c = getchar();

        if( c == EOF ) break;

        if( (i % 16) == 0 )
            printf("\n\t");

        printf( "0x%02x, ",c);
    }

    printf("\n};\n");

    exit(0);
}

//==========================================================================
// End of entable.c
