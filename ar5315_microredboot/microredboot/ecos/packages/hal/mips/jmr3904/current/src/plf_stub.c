//=============================================================================
//
//      plf_stub.c
//
//      Platform specific code for GDB stub support.
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg, jskov (based on the old tx39 hal_stub.c)
// Contributors:nickg, jskov
// Date:        1999-02-12
// Purpose:     Platform specific code for GDB stub support.
//              
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#include <cyg/hal/hal_stub.h>

#include <cyg/hal/hal_io.h>             // HAL IO macros
#include <cyg/hal/hal_diag.h>           // diag output. FIXME

//-----------------------------------------------------------------------------
// Serial definitions.
#define DIAG_BASE       0xfffff300
#define DIAG_SLCR       (DIAG_BASE+0x00)
#define DIAG_SLSR       (DIAG_BASE+0x04)
#define DIAG_SLDICR     (DIAG_BASE+0x08)
#define DIAG_SLDISR     (DIAG_BASE+0x0C)
#define DIAG_SFCR       (DIAG_BASE+0x10)
#define DIAG_SBRG       (DIAG_BASE+0x14)
#define DIAG_TFIFO      (DIAG_BASE+0x20)
#define DIAG_RFIFO      (DIAG_BASE+0x30)

#define BRG_T0          0x0000
#define BRG_T2          0x0100
#define BRG_T4          0x0200
#define BRG_T5          0x0300

//-----------------------------------------------------------------------------

// Initialize the current serial port.
void hal_jmr_init_serial( void )
{
//hal_diag_led(0x10);    
    HAL_WRITE_UINT16( DIAG_SLCR , 0x0020 );

    HAL_WRITE_UINT16( DIAG_SLDICR , 0x0000 );
    
    HAL_WRITE_UINT16( DIAG_SFCR , 0x0000 );
    
//    HAL_WRITE_UINT16( DIAG_SBRG , BRG_T2 | 20 );
//    HAL_WRITE_UINT16( DIAG_SBRG , BRG_T2 | 10 );
    HAL_WRITE_UINT16( DIAG_SBRG , BRG_T2 | 5 );    

//hal_diag_led(0x10);
}

// Write C to the current serial port.
void hal_jmr_put_char( int c )
{
    CYG_WORD16 disr;
    
//hal_diag_led(0x20);

    for(;;)
    {
        HAL_READ_UINT16( DIAG_SLDISR , disr );

        if( disr & 0x0002 ) break;
    }

    disr = disr & ~0x0002;
    
    HAL_WRITE_UINT8( DIAG_TFIFO, c );

    HAL_WRITE_UINT16( DIAG_SLDISR , disr );    

//hal_diag_led(0x20);
//    HAL_DIAG_WRITE_CHAR( c );
}

// Read one character from the current serial port.
int hal_jmr_get_char( void )
{
#ifdef CYGPKG_HAL_MIPS_SIM
    // FIXME: ask nickg if this can go to /dev/null
    static char input[] = "+$s#73";
    static int i = 0;
    return input[i++];
#else
    char c;

    CYG_WORD16 disr;

//hal_diag_led(0x40);        
    for(;;)
    {
        
        HAL_READ_UINT16( DIAG_SLDISR , disr );

        if( disr & 0x0001 ) break;
    }

    disr = disr & ~0x0001;
    
    HAL_READ_UINT8( DIAG_RFIFO, c );
    
    HAL_WRITE_UINT16( DIAG_SLDISR , disr );    

//hal_diag_led(0x40);
    
//    HAL_DIAG_READ_CHAR(c);
//    diag_printf("<%02x:%c>",c);

    return c;
    
#endif    
}

#endif // ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
//-----------------------------------------------------------------------------
// End of plf_stub.c
