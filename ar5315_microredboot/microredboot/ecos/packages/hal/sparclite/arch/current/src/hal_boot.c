//==========================================================================
//
//      hal_boot.c
//
//      SPARClite Architecture specific interrupt dispatch tables
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
// Author(s):    hmt
// Contributors: hmt
// Date:         1998-12-10
// Purpose:      Interrupt handler tables for SPARClite.
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

/*------------------------------------------------------------------------*/
/* calling this is our raison d'etre: */
extern void cyg_start( void );

/*------------------------------------------------------------------------*/
/* data copy and bss zero functions                                       */

typedef void (CYG_ROM_ADDRESS)(void);

#ifdef CYG_HAL_STARTUP_ROM      
void hal_copy_data(void)
{
    extern char __ram_data_start;
    extern char __ram_data_end;
    extern CYG_ROM_ADDRESS __rom_data_start;    
    long *p = (long *)&__ram_data_start;
    long *q = (long *)&__rom_data_start;
    
    while( p <= (long *)&__ram_data_end )
        *p++ = *q++;
}
#endif

void hal_zero_bss(void)
{
    extern CYG_ROM_ADDRESS __bss_start;
    extern CYG_ROM_ADDRESS __bss_end;

    register long long zero = 0;
    register long long *end = (long long *)&__bss_end;
    register long long *p = (long long *)&__bss_start;

    while( p <= end )
        *p++ = zero;   
}

#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
cyg_bool cyg_hal_stop_constructors;
#endif

void
cyg_hal_invoke_constructors (void)
{
    typedef void (*pfunc) (void);
    extern pfunc __CTOR_LIST__[];
    extern pfunc __CTOR_END__[];

#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    static pfunc *p = &__CTOR_END__[-1];
    
    cyg_hal_stop_constructors = 0;
    for (; p >= __CTOR_LIST__; p--) {
        (*p) ();
        if (cyg_hal_stop_constructors) {
            p--;
            break;
        }
    }
#else
    pfunc *p;

    for (p = &__CTOR_END__[-1]; p >= __CTOR_LIST__; p--)
        (*p) ();
#endif
}

// Override any __gccmain the compiler might generate. We don't want
// constructors to be called twice.
void  __gccmain(void) {}

/*------------------------------------------------------------------------*/
/*   CYG_HAL_START - pre-main-entrypoint                                  */

#ifdef CYGPKG_HAL_SPARCLITE_SLEB
#define SLEB_LED (*(volatile char *)(0x02000003))
#define LED( _x_ ) SLEB_LED = (char)(0xff & ~(_x_))
#else
#define LED( _x_ ) CYG_EMPTY_STATEMENT
#endif

extern void hal_board_prestart( void );
extern void hal_board_poststart( void );

// This is called with traps enabled, but interrupts masked out:
// Be sure to enable them in hal_board_poststart() at the latest.

void cyg_hal_start( void )
{
    /* Board specific prestart that's best done in C */
    hal_board_prestart();

    LED( 0xd0 );

#ifdef CYG_HAL_STARTUP_ROM
    /* Copy data from ROM to RAM */
    hal_copy_data();
#endif
                
    LED( 0xd4 );

    /* Zero BSS */
    hal_zero_bss();

    LED( 0xd8 );

    /* Call constructors */
    cyg_hal_invoke_constructors();

    LED( 0xdc );

    /* Board specific late startup that's best done in C */
    hal_board_poststart();

    LED( 0xf8 );

    /* Call cyg_start */
    cyg_start(); /* does not return */
}

// EOF hal_boot.c
