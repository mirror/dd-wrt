//===========================================================================
//
//      s_rint.c
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


/* @(#)s_rint.c 1.3 95/01/18 */
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

/*
 * rint(x)
 * Return x rounded to integral value according to the prevailing
 * rounding mode.
 * Method:
 *      Using floating addition.
 * Exception:
 *      Inexact flag raised if x not equal to rint(x).
 */

#include "mathincl/fdlibm.h"

static const double
TWO52[2]={
  4.50359962737049600000e+15, /* 0x43300000, 0x00000000 */
 -4.50359962737049600000e+15, /* 0xC3300000, 0x00000000 */
};

        double rint(double x)
{
        int i0,j0,sx;
        unsigned i,i1;
        double w,t;
        i0 =  CYG_LIBM_HI(x);
        sx = (i0>>31)&1;
        i1 =  CYG_LIBM_LO(x);
        j0 = ((i0>>20)&0x7ff)-0x3ff;
        if(j0<20) {
            if(j0<0) {  
                if(((i0&0x7fffffff)|i1)==0) return x;
                i1 |= (i0&0x0fffff);
                i0 &= 0xfffe0000;
                i0 |= ((i1|-i1)>>12)&0x80000;
                CYG_LIBM_HI(x)=i0;
                w = TWO52[sx]+x;
                t =  w-TWO52[sx];
                i0 = CYG_LIBM_HI(t);
                CYG_LIBM_HI(t) = (i0&0x7fffffff)|(sx<<31);
                return t;
            } else {
                i = (0x000fffff)>>j0;
                if(((i0&i)|i1)==0) return x; /* x is integral */
                i>>=1;
                if(((i0&i)|i1)!=0) {
                    if(j0==19) i1 = 0x40000000; else
                    i0 = (i0&(~i))|((0x20000)>>j0);
                }
            }
        } else if (j0>51) {
            if(j0==0x400) return x+x;   /* inf or NaN */
            else return x;              /* x is integral */
        } else {
            i = ((unsigned)(0xffffffff))>>(j0-20);
            if((i1&i)==0) return x;     /* x is integral */
            i>>=1;
            if((i1&i)!=0) i1 = (i1&(~i))|((0x40000000)>>(j0-20));
        }
        CYG_LIBM_HI(x) = i0;
        CYG_LIBM_LO(x) = i1;
        w = TWO52[sx]+x;
        return w-TWO52[sx];
}

#endif // ifdef CYGPKG_LIBM     

// EOF s_rint.c
