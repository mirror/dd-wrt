//===========================================================================
//
//      s_cbrt.c
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


/* @(#)s_cbrt.c 1.3 95/01/18 */
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

#include "mathincl/fdlibm.h"

/* cbrt(x)
 * Return cube root of x
 */
static const unsigned 
        B1 = 715094163, /* B1 = (682-0.03306235651)*2**20 */
        B2 = 696219795; /* B2 = (664-0.03306235651)*2**20 */

static const double
C =  5.42857142857142815906e-01, /* 19/35     = 0x3FE15F15, 0xF15F15F1 */
D = -7.05306122448979611050e-01, /* -864/1225 = 0xBFE691DE, 0x2532C834 */
E =  1.41428571428571436819e+00, /* 99/70     = 0x3FF6A0EA, 0x0EA0EA0F */
F =  1.60714285714285720630e+00, /* 45/28     = 0x3FF9B6DB, 0x6DB6DB6E */
G =  3.57142857142857150787e-01; /* 5/14      = 0x3FD6DB6D, 0xB6DB6DB7 */

        double cbrt(double x) 
{
        int     hx;
        double r,s,t=0.0,w;
        unsigned sign;


        hx = CYG_LIBM_HI(x);            /* high word of x */
        sign=hx&0x80000000;             /* sign= sign(x) */
        hx  ^=sign;
        if(hx>=0x7ff00000) return(x+x); /* cbrt(NaN,INF) is itself */
        if((hx|CYG_LIBM_LO(x))==0) 
            return(x);          /* cbrt(0) is itself */

        CYG_LIBM_HI(x) = hx;    /* x <- |x| */
    /* rough cbrt to 5 bits */
        if(hx<0x00100000)               /* subnormal number */
          {CYG_LIBM_HI(t)=0x43500000;           /* set t= 2**54 */
           t*=x; CYG_LIBM_HI(t)=CYG_LIBM_HI(t)/3+B2;
          }
        else
          CYG_LIBM_HI(t)=hx/3+B1;       


    /* new cbrt to 23 bits, may be implemented in single precision */
        r=t*t/x;
        s=C+r*t;
        t*=G+F/(s+E+D/s);       

    /* chopped to 20 bits and make it larger than cbrt(x) */ 
        CYG_LIBM_LO(t)=0; CYG_LIBM_HI(t)+=0x00000001;


    /* one step newton iteration to 53 bits with error less than 0.667 ulps */
        s=t*t;          /* t*t is exact */
        r=x/s;
        w=t+t;
        r=(r-t)/(w+r);  /* r-s is exact */
        t=t+t*r;

    /* retore the sign bit */
        CYG_LIBM_HI(t) |= sign;
        return(t);
}

#endif // ifdef CYGPKG_LIBM     

// EOF s_cbrt.c
