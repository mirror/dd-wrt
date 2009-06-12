#ifndef CYGONCE_LIBC_STDLIB_DIV_INL
#define CYGONCE_LIBC_STDLIB_DIV_INL
/*===========================================================================
//
//      div.inl
//
//      Inline implementations for the ISO standard utility functions
//      div() and ldiv()
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: 
// Date:         2000-04-28
// Purpose:     
// Description: 
// Usage:        Do not include this file directly - include <stdlib.h> instead
//
//####DESCRIPTIONEND####
//
//=========================================================================*/

// CONFIGURATION

#include <pkgconf/libc_stdlib.h>    // Configuration header

// INCLUDES

#include <cyg/infra/cyg_ass.h>      // Assertion support
#include <cyg/infra/cyg_trac.h>     // Tracing support

/* TYPE DEFINITIONS */

/* return type of the div() function */

typedef struct {
    int quot;      /* quotient  */
    int rem;       /* remainder */
} div_t;


/* return type of the ldiv() function */

typedef struct {
    long quot;     /* quotient  */
    long rem;      /* remainder */
} ldiv_t;

/* FUNCTION PROTOTYPES */

#ifdef __cplusplus
extern "C" {
#endif

extern div_t
div( int /* numerator */, int /* denominator */ ) __attribute__((__const__));

extern ldiv_t
ldiv( long /* numerator */, long /* denominator */ ) __attribute__((__const__));

#ifdef __cplusplus
} /* extern "C" */
#endif 

/* FUNCTIONS */

#ifndef CYGPRI_LIBC_STDLIB_DIV_INLINE
# define CYGPRI_LIBC_STDLIB_DIV_INLINE extern __inline__
#endif

CYGPRI_LIBC_STDLIB_DIV_INLINE div_t
div( int __numer, int __denom )
{
    div_t __ret;

    CYG_REPORT_FUNCNAMETYPE( "div", "quotient: %d");
    CYG_REPORT_FUNCARG2DV( __numer, __denom );
    // FIXME: what if they want it handled with SIGFPE? Should have option
    CYG_PRECONDITION(__denom != 0, "division by zero attempted!");
    
    __ret.quot = __numer / __denom;
    __ret.rem  = __numer % __denom;

    // But the modulo is implementation-defined for -ve numbers (ISO C 6.3.5)
    // and we are required to "round" to zero (ISO C 7.10.6.2)
    //
    // The cases we have to deal with are inexact division of:
    // a) + div +
    // b) + div -
    // c) - div +
    // d) - div -
    //
    // a) can never go wrong and the quotient and remainder are always positive
    // b) only goes wrong if the negative quotient has been "rounded" to
    //    -infinity - if so then the remainder will be negative when it
    //    should be positive or zero
    // c) only goes wrong if the negative quotient has been "rounded" to
    //    -infinity - if so then the remainder will be positive when it
    //    should be negative or zero
    // d) only goes wrong if the positive quotient has been rounded to
    //    +infinity - if so then the remainder will be positive when it
    //    should be negative or zero
    //
    // So the correct sign of the remainder corresponds to the sign of the
    // numerator. Which means we can say that the result needs adjusting
    // iff the sign of the numerator is different from the sign of the
    // remainder.
    //
    // You may be interested to know that the Berkeley version of div()
    // would get this wrong for e.g. (c) and (d) on some targets.
    // e.g. for (-5)/4 it could leave the result as -2R3

    if ((__ret.rem < 0) && (__numer > 0)) {
        ++__ret.quot;
        __ret.rem -= __denom;
    } else if ((__ret.rem > 0) && (__numer < 0)) {
        --__ret.quot;
        __ret.rem += __denom;
    } // else

    CYG_REPORT_RETVAL( __ret.quot );

    return __ret;
} // div()

CYGPRI_LIBC_STDLIB_DIV_INLINE ldiv_t
ldiv( long __numer, long __denom )
{
    ldiv_t __ret;

    CYG_REPORT_FUNCNAMETYPE( "ldiv", "quotient: %d");
    CYG_REPORT_FUNCARG2DV( __numer, __denom );
    // FIXME: what if they want it handled with SIGFPE? Should have option
    CYG_PRECONDITION(__denom != 0, "division by zero attempted!");
    
    __ret.quot = __numer / __denom;
    __ret.rem  = __numer % __denom;

    // But the modulo is implementation-defined for -ve numbers (ISO C 6.3.5)
    // and we are required to "round" to zero (ISO C 7.10.6.2)
    //
    // The cases we have to deal with are inexact division of:
    // a) + div +
    // b) + div -
    // c) - div +
    // d) - div -
    //
    // a) can never go wrong and the quotient and remainder are always positive
    // b) only goes wrong if the negative quotient has been "rounded" to
    //    -infinity - if so then the remainder will be negative when it
    //    should be positive or zero
    // c) only goes wrong if the negative quotient has been "rounded" to
    //    -infinity - if so then the remainder will be positive when it
    //    should be negative or zero
    // d) only goes wrong if the positive quotient has been rounded to
    //    +infinity - if so then the remainder will be positive when it
    //    should be negative or zero
    //
    // So the correct sign of the remainder corresponds to the sign of the
    // numerator. Which means we can say that the result needs adjusting
    // iff the sign of the numerator is different from the sign of the
    // remainder.
    //
    // You may be interested to know that the Berkeley version of ldiv()
    // would get this wrong for e.g. (c) and (d) on some targets.
    // e.g. for (-5)/4 it could leave the result as -2R3

    if ((__ret.rem < 0) && (__numer > 0)) {
        ++__ret.quot;
        __ret.rem -= __denom;
    } else if ((__ret.rem > 0) && (__numer < 0)) {
        --__ret.quot;
        __ret.rem += __denom;
    } // else

    CYG_REPORT_RETVAL( __ret.quot );

    return __ret;
} // ldiv()


#endif // CYGONCE_LIBC_STDLIB_DIV_INL multiple inclusion protection

// EOF div.inl
