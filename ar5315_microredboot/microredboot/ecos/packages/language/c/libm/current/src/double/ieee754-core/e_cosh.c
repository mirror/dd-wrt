//===========================================================================
//
//      e_cosh.c
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


/* @(#)e_cosh.c 1.3 95/01/18 */
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

/* __ieee754_cosh(x)
 * Method : 
 * mathematically cosh(x) if defined to be (exp(x)+exp(-x))/2
 *      1. Replace x by |x| (cosh(x) = cosh(-x)). 
 *      2. 
 *                                                      [ exp(x) - 1 ]^2 
 *          0        <= x <= ln2/2  :  cosh(x) := 1 + -------------------
 *                                                         2*exp(x)
 *
 *                                                exp(x) +  1/exp(x)
 *          ln2/2    <= x <= 22     :  cosh(x) := -------------------
 *                                                        2
 *          22       <= x <= lnovft :  cosh(x) := exp(x)/2 
 *          lnovft   <= x <= ln2ovft:  cosh(x) := exp(x/2)/2 * exp(x/2)
 *          ln2ovft  <  x           :  cosh(x) := huge*huge (overflow)
 *
 * Special cases:
 *      cosh(x) is |x| if x is +INF, -INF, or NaN.
 *      only cosh(0)=1 is exact for finite x.
 */

#include "mathincl/fdlibm.h"

static const double one = 1.0, half=0.5, huge = 1.0e300;

        double __ieee754_cosh(double x)
{       
        double t,w;
        int ix;
        unsigned lx;

    /* High word of |x|. */
        ix = CYG_LIBM_HI(x);
        ix &= 0x7fffffff;

    /* x is INF or NaN */
        if(ix>=0x7ff00000) return x*x;  

    /* |x| in [0,0.5*ln2], return 1+expm1(|x|)^2/(2*exp(|x|)) */
        if(ix<0x3fd62e43) {
            t = expm1(fabs(x));
            w = one+t;
            if (ix<0x3c800000) return w;        /* cosh(tiny) = 1 */
            return one+(t*t)/(w+w);
        }

    /* |x| in [0.5*ln2,22], return (exp(|x|)+1/exp(|x|)/2; */
        if (ix < 0x40360000) {
                t = __ieee754_exp(fabs(x));
                return half*t+half/t;
        }

    /* |x| in [22, log(maxdouble)] return half*exp(|x|) */
        if (ix < 0x40862E42)  return half*__ieee754_exp(fabs(x));

    /* |x| in [log(maxdouble), overflowthresold] */
        lx = CYG_LIBM_LO(x);
        if (ix<0x408633CE || 
              ((ix==0x408633ce)&&(lx<=(unsigned)0x8fb9f87d))) {
            w = __ieee754_exp(half*fabs(x));
            t = half*w;
            return t*w;
        }

    /* |x| > overflowthresold, cosh(x) overflow */
        return huge*huge;
}

#endif // ifdef CYGPKG_LIBM     

// EOF e_cosh.c
