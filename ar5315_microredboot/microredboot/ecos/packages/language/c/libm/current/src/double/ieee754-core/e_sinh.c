//===========================================================================
//
//      e_sinh.c
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


/* @(#)e_sinh.c 1.3 95/01/18 */
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

/* __ieee754_sinh(x)
 * Method : 
 * mathematically sinh(x) if defined to be (exp(x)-exp(-x))/2
 *      1. Replace x by |x| (sinh(-x) = -sinh(x)). 
 *      2. 
 *                                                  E + E/(E+1)
 *          0        <= x <= 22     :  sinh(x) := --------------, E=expm1(x)
 *                                                      2
 *
 *          22       <= x <= lnovft :  sinh(x) := exp(x)/2 
 *          lnovft   <= x <= ln2ovft:  sinh(x) := exp(x/2)/2 * exp(x/2)
 *          ln2ovft  <  x           :  sinh(x) := x*shuge (overflow)
 *
 * Special cases:
 *      sinh(x) is |x| if x is +INF, -INF, or NaN.
 *      only sinh(0)=0 is exact for finite x.
 */

#include "mathincl/fdlibm.h"

static const double one = 1.0, shuge = 1.0e307;

        double __ieee754_sinh(double x)
{       
        double t,w,h;
        int ix,jx;
        unsigned lx;

    /* High word of |x|. */
        jx = CYG_LIBM_HI(x);
        ix = jx&0x7fffffff;

    /* x is INF or NaN */
        if(ix>=0x7ff00000) return x+x;  

        h = 0.5;
        if (jx<0) h = -h;
    /* |x| in [0,22], return sign(x)*0.5*(E+E/(E+1))) */
        if (ix < 0x40360000) {          /* |x|<22 */
            if (ix<0x3e300000)          /* |x|<2**-28 */
                if(shuge+x>one) return x;/* sinh(tiny) = tiny with inexact */
            t = expm1(fabs(x));
            if(ix<0x3ff00000) return h*(2.0*t-t*t/(t+one));
            return h*(t+t/(t+one));
        }

    /* |x| in [22, log(maxdouble)] return 0.5*exp(|x|) */
        if (ix < 0x40862E42)  return h*__ieee754_exp(fabs(x));

    /* |x| in [log(maxdouble), overflowthresold] */
        lx = CYG_LIBM_LO(x);
        if (ix<0x408633CE || ((ix==0x408633ce)&&(lx<=(unsigned)0x8fb9f87d))) {
            w = __ieee754_exp(0.5*fabs(x));
            t = h*w;
            return t*w;
        }

    /* |x| > overflowthresold, sinh(x) overflow */
        return x*shuge;
}

#endif // ifdef CYGPKG_LIBM     

// EOF e_sinh.c
