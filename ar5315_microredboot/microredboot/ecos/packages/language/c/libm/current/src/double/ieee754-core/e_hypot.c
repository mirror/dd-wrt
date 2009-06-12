//===========================================================================
//
//      e_hypot.c
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


/* @(#)e_hypot.c 1.3 95/01/18 */
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

/* __ieee754_hypot(x,y)
 *
 * Method :                  
 *      If (assume round-to-nearest) z=x*x+y*y 
 *      has error less than sqrt(2)/2 ulp, than 
 *      sqrt(z) has error less than 1 ulp (exercise).
 *
 *      So, compute sqrt(x*x+y*y) with some care as 
 *      follows to get the error below 1 ulp:
 *
 *      Assume x>y>0;
 *      (if possible, set rounding to round-to-nearest)
 *      1. if x > 2y  use
 *              x1*x1+(y*y+(x2*(x+x1))) for x*x+y*y
 *      where x1 = x with lower 32 bits cleared, x2 = x-x1; else
 *      2. if x <= 2y use
 *              t1*y1+((x-y)*(x-y)+(t1*y2+t2*y))
 *      where t1 = 2x with lower 32 bits cleared, t2 = 2x-t1, 
 *      y1= y with lower 32 bits chopped, y2 = y-y1.
 *              
 *      NOTE: scaling may be necessary if some argument is too 
 *            large or too tiny
 *
 * Special cases:
 *      hypot(x,y) is INF if x or y is +INF or -INF; else
 *      hypot(x,y) is NAN if x or y is NAN.
 *
 * Accuracy:
 *      hypot(x,y) returns sqrt(x^2+y^2) with error less 
 *      than 1 ulps (units in the last place) 
 */

#include "mathincl/fdlibm.h"

        double __ieee754_hypot(double x, double y)
{
        double a=x,b=y,t1,t2,y1,y2,w;
        int j,k,ha,hb;

        ha = CYG_LIBM_HI(x)&0x7fffffff; /* high word of  x */
        hb = CYG_LIBM_HI(y)&0x7fffffff; /* high word of  y */
        if(hb > ha) {a=y;b=x;j=ha; ha=hb;hb=j;} else {a=x;b=y;}
        CYG_LIBM_HI(a) = ha;    /* a <- |a| */
        CYG_LIBM_HI(b) = hb;    /* b <- |b| */
        if((ha-hb)>0x3c00000) {return a+b;} /* x/y > 2**60 */
        k=0;
        if(ha > 0x5f300000) {   /* a>2**500 */
           if(ha >= 0x7ff00000) {       /* Inf or NaN */
               w = a+b;                 /* for sNaN */
               if(((ha&0xfffff)|CYG_LIBM_LO(a))==0) w = a;
               if(((hb^0x7ff00000)|CYG_LIBM_LO(b))==0) w = b;
               return w;
           }
           /* scale a and b by 2**-600 */
           ha -= 0x25800000; hb -= 0x25800000;  k += 600;
           CYG_LIBM_HI(a) = ha;
           CYG_LIBM_HI(b) = hb;
        }
        if(hb < 0x20b00000) {   /* b < 2**-500 */
            if(hb <= 0x000fffff) {      /* subnormal b or 0 */  
                if((hb|(CYG_LIBM_LO(b)))==0) return a;
                t1=0;
                CYG_LIBM_HI(t1) = 0x7fd00000;   /* t1=2^1022 */
                b *= t1;
                a *= t1;
                k -= 1022;
            } else {            /* scale a and b by 2^600 */
                ha += 0x25800000;       /* a *= 2^600 */
                hb += 0x25800000;       /* b *= 2^600 */
                k -= 600;
                CYG_LIBM_HI(a) = ha;
                CYG_LIBM_HI(b) = hb;
            }
        }
    /* medium size a and b */
        w = a-b;
        if (w>b) {
            t1 = 0;
            CYG_LIBM_HI(t1) = ha;
            t2 = a-t1;
            w  = sqrt(t1*t1-(b*(-b)-t2*(a+t1)));
        } else {
            a  = a+a;
            y1 = 0;
            CYG_LIBM_HI(y1) = hb;
            y2 = b - y1;
            t1 = 0;
            CYG_LIBM_HI(t1) = ha+0x00100000;
            t2 = a - t1;
            w  = sqrt(t1*y1-(w*(-w)-(t1*y2+t2*b)));
        }
        if(k!=0) {
            t1 = 1.0;
            CYG_LIBM_HI(t1) += (k<<20);
            return t1*w;
        } else return w;
}

#endif // ifdef CYGPKG_LIBM     

// EOF e_hypot.c
