//===========================================================================
//
//      s_tanh.c
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


/* @(#)s_tanh.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

/* Tanh(x)
 * Return the Hyperbolic Tangent of x
 *
 * Method :
 *                                     x    -x
 *                                    e  - e
 *      0. tanh(x) is defined to be -----------
 *                                     x    -x
 *                                    e  + e
 *      1. reduce x to non-negative by tanh(-x) = -tanh(x).
 *      2.  0      <= x <= 2**-55 : tanh(x) := x*(one+x)
 *                                              -t
 *          2**-55 <  x <=  1     : tanh(x) := -----; t = expm1(-2x)
 *                                             t + 2
 *                                                   2
 *          1      <= x <=  22.0  : tanh(x) := 1-  ----- ; t=expm1(2x)
 *                                                 t + 2
 *          22.0   <  x <= INF    : tanh(x) := 1.
 *
 * Special cases:
 *      tanh(NaN) is NaN;
 *      only tanh(0)=0 is exact for finite argument.
 */

#include "mathincl/fdlibm.h"

static const double one=1.0, two=2.0, tiny = 1.0e-300;

        double tanh(double x)
{
        double t,z;
        int jx,ix;

    /* High word of |x|. */
        jx = CYG_LIBM_HI(x);
        ix = jx&0x7fffffff;

    /* x is INF or NaN */
        if(ix>=0x7ff00000) { 
            if (jx>=0) return one/x+one;    /* tanh(+-inf)=+-1 */
            else       return one/x-one;    /* tanh(NaN) = NaN */
        }

    /* |x| < 22 */
        if (ix < 0x40360000) {          /* |x|<22 */
            if (ix<0x3c800000)          /* |x|<2**-55 */
                return x*(one+x);       /* tanh(small) = small */
            if (ix>=0x3ff00000) {       /* |x|>=1  */
                t = expm1(two*fabs(x));
                z = one - two/(t+two);
            } else {
                t = expm1(-two*fabs(x));
                z= -t/(t+two);
            }
    /* |x| > 22, return +-1 */
        } else {
            z = one - tiny;             /* raised inexact flag */
        }
        return (jx>=0)? z: -z;
}

#endif // ifdef CYGPKG_LIBM     

// EOF s_tanh.c
