//===========================================================================
//
//      e_acosh.c
//
//      Part of the standard mathematical function library
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
// Author(s):   jlarmour
// Contributors:  jlarmour
// Date:        1998-02-13
// Purpose:     
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libm.h>   // Configuration header

// Include the Math library?
#ifdef CYGPKG_LIBM     

// Derived from code with the following copyright


/* @(#)e_acosh.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 *
 */

/* __ieee754_acosh(x)
 * Method :
 *      Based on 
 *              acosh(x) = log [ x + sqrt(x*x-1) ]
 *      we have
 *              acosh(x) := log(x)+ln2, if x is large; else
 *              acosh(x) := log(2x-1/(sqrt(x*x-1)+x)) if x>2; else
 *              acosh(x) := log1p(t+sqrt(2.0*t+t*t)); where t=x-1.
 *
 * Special cases:
 *      acosh(x) is NaN with signal if x<1.
 *      acosh(NaN) is NaN without signal.
 */

#include "mathincl/fdlibm.h"

static const double 
one     = 1.0,
ln2     = 6.93147180559945286227e-01;  /* 0x3FE62E42, 0xFEFA39EF */

        double __ieee754_acosh(double x)
{       
        double t;
        int hx;
        hx = CYG_LIBM_HI(x);
        if(hx<0x3ff00000) {             /* x < 1 */
            return (x-x)/(x-x);
        } else if(hx >=0x41b00000) {    /* x > 2**28 */
            if(hx >=0x7ff00000) {       /* x is inf of NaN */
                return x+x;
            } else 
                return __ieee754_log(x)+ln2;    /* acosh(huge)=log(2x) */
        } else if(((hx-0x3ff00000)|CYG_LIBM_LO(x))==0) {
            return 0.0;                 /* acosh(1) = 0 */
        } else if (hx > 0x40000000) {   /* 2**28 > x > 2 */
            t=x*x;
            return __ieee754_log(2.0*x-one/(x+sqrt(t-one)));
        } else {                        /* 1<x<2 */
            t = x-one;
            return log1p(t+sqrt(2.0*t+t*t));
        }
}

#endif // ifdef CYGPKG_LIBM     

// EOF e_acosh.c
