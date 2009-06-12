//===========================================================================
//
//      strtod.cxx
//
//      ISO String to double-precision floating point conversion
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
// Date:         2000-04-30
// Purpose:     
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdlib.h>           // Configuration header

// Include strtod()?
#if defined(CYGFUN_LIBC_strtod)

// INCLUDES

#include <cyg/infra/cyg_type.h>     // Common type definitions and support
#include <cyg/infra/cyg_trac.h>     // Tracing support
#include <cyg/infra/cyg_ass.h>      // Assertion support
#include <stddef.h>                 // NULL, wchar_t and size_t from compiler
#include <stdlib.h>                 // Main header for stdlib functions
#include <ctype.h>                  // isspace() and isdigit()
#include <float.h>                  // DBL_MIN_10_EXP and DBL_MAX_10_EXP
#include <math.h>                   // HUGE_VAL
#include <errno.h>                  // errno

// CONSTANTS

#define MAXE (DBL_MAX_10_EXP)
#define MINE (DBL_MIN_10_EXP)

// flags
#define SIGN    0x01
#define ESIGN   0x02
#define DECP    0x04


// MACROS

#define Ise(c)          ((c == 'e') || (c == 'E') || (c == 'd') || (c == 'D'))
#define Issign(c)       ((c == '-') || (c == '+'))
#define Val(c)          ((c - '0'))


// FUNCTIONS

/*
 * [atw] multiply 64 bit accumulator by 10 and add digit.
 * The KA/CA way to do this should be to use
 * a 64-bit integer internally and use "adjust" to
 * convert it to float at the end of processing.
 */
static int
ten_mul(double *acc, int digit)
{
    /* [atw] Crude, but effective (at least on a KB)...
     */
    *acc *= 10;
    *acc += digit;
    
    return 0;     /* no overflow */
} // ten_mul()


/*
 * compute 10**x by successive squaring.
 */

static const double
exp10(unsigned x)
{
    static double powtab[] = {1.0,
                              10.0,
                              100.0,
                              1000.0,
                              10000.0};
    
    if (x < (sizeof(powtab)/sizeof(double)))
        return powtab[x];
    else if (x & 1)
        return 10.0 * exp10(x-1);
    else
        return exp10(x/2) * exp10(x/2);
} // exp10()


/*
 * return (*acc) scaled by 10**dexp.
 */

static double
adjust(double *acc, int dexp, int sign)
     /* *acc    the 64 bit accumulator */
     /* dexp    decimal exponent       */
     /* sign    sign flag              */
{
    double r;
    
    if (dexp > MAXE)
    {
        errno = ERANGE;
        return (sign) ? -HUGE_VAL : HUGE_VAL;
    }
    else if (dexp < MINE)
    {
        errno = ERANGE;
        return 0.0;
    }
    
    r = *acc;
    if (sign)
        r = -r;
    if (dexp==0)
        return r;
    
    if (dexp < 0)
        return r / exp10(abs(dexp));
    else
        return r * exp10(dexp);
} // adjust()


externC double
strtod( const char *nptr, char **endptr )
{
    const char *start=nptr;
    double accum = 0.0;
    int flags = 0;
    int texp  = 0;
    int e     = 0;
    int conv_done = 0;
  
    double retval;

    CYG_REPORT_FUNCNAMETYPE( "strtod", "returning %f" );

    CYG_CHECK_DATA_PTR( nptr, "nptr is an invalid pointer!" );

    // endptr is allowed to be NULL, but if it isn't, we check it
    if (endptr != NULL)
        CYG_CHECK_DATA_PTR( endptr, "endptr is an invalid pointer!" );
    
    while(isspace(*nptr)) nptr++;
    if(*nptr == '\0')
    {   /* just leading spaces */
        if(endptr != NULL) *endptr = (char *)start;
        return 0.0;
    }
    
    
    if(Issign(*nptr))
    {
        if(*nptr == '-') flags = SIGN;
        if(*++nptr == '\0')
        {   /* "+|-" : should be an error ? */
            if(endptr != NULL) *endptr = (char *)start;
            return 0.0;
        }
    }
    
    for(; (isdigit(*nptr) || (*nptr == '.')); nptr++)
    {
        conv_done = 1;
        if(*nptr == '.')
            flags |= DECP;
        else
        {
            if( ten_mul(&accum, Val(*nptr)) ) texp++;
            if(flags & DECP) texp--;
        }
    }
    
    if(Ise(*nptr))
    {
        conv_done = 1;
        if(*++nptr != '\0') /* skip e|E|d|D */
        {  /* ! ([nptr]xxx[.[yyy]]e)  */
            
            while(isspace(*nptr)) nptr++; /* Ansi allows spaces after e */
            if(*nptr != '\0')
            { /*  ! ([nptr]xxx[.[yyy]]e[space])  */
                
                if(Issign(*nptr))
                    if(*nptr++ == '-') flags |= ESIGN;
                
                if(*nptr != '\0')
                { /*  ! ([nptr]xxx[.[yyy]]e[nptr])  -- error?? */
                    
                    for(; isdigit(*nptr); nptr++)
                        if (e < MAXE) /* prevent from grossly overflowing */
                            e = e*10 + Val(*nptr);
                    
                    /* dont care what comes after this */
                    if(flags & ESIGN)
                        texp -= e;
                    else
                        texp += e;
                }
            }
        }
    }
    
    if(endptr != NULL) 
        *endptr = (char *)((conv_done) ? nptr : start);
    
    retval = adjust(&accum, (int)texp, (int)(flags & SIGN));
  

    CYG_REPORT_RETVAL( retval );

    return retval;
} // strtod()


#endif // if defined(CYGFUN_LIBC_strtod)

// EOF strtod.cxx
