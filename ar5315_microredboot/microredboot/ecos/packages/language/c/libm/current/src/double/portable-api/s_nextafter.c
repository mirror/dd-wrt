//===========================================================================
//
//      s_nextafter.c
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


/* @(#)s_nextafter.c 1.3 95/01/18 */
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

/* IEEE functions
 *      nextafter(x,y)
 *      return the next machine floating-point number of x in the
 *      direction toward y.
 *   Special cases:
 */

#include "mathincl/fdlibm.h"

        double nextafter(double x, double y)
{
        int     hx,hy,ix,iy;
        unsigned lx,ly;

        hx = CYG_LIBM_HI(x);            /* high word of x */
        lx = CYG_LIBM_LO(x);            /* low  word of x */
        hy = CYG_LIBM_HI(y);            /* high word of y */
        ly = CYG_LIBM_LO(y);            /* low  word of y */
        ix = hx&0x7fffffff;             /* |x| */
        iy = hy&0x7fffffff;             /* |y| */

        if(((ix>=0x7ff00000)&&((ix-0x7ff00000)|lx)!=0) ||   /* x is nan */ 
           ((iy>=0x7ff00000)&&((iy-0x7ff00000)|ly)!=0))     /* y is nan */ 
           return x+y;                          
        if(x==y) return x;              /* x=y, return x */
        if((ix|lx)==0) {                        /* x == 0 */
            CYG_LIBM_HI(x) = hy&0x80000000;     /* return +-minsubnormal */
            CYG_LIBM_LO(x) = 1;
            y = x*x;
            if(y==x) return y; else return x;   /* raise underflow flag */
        } 
        if(hx>=0) {                             /* x > 0 */
            if(hx>hy||((hx==hy)&&(lx>ly))) {    /* x > y, x -= ulp */
                if(lx==0) hx -= 1;
                lx -= 1;
            } else {                            /* x < y, x += ulp */
                lx += 1;
                if(lx==0) hx += 1;
            }
        } else {                                /* x < 0 */
            if(hy>=0||hx>hy||((hx==hy)&&(lx>ly))){/* x < y, x -= ulp */
                if(lx==0) hx -= 1;
                lx -= 1;
            } else {                            /* x > y, x += ulp */
                lx += 1;
                if(lx==0) hx += 1;
            }
        }
        hy = hx&0x7ff00000;
        if(hy>=0x7ff00000) return x+x;  /* overflow  */
        if(hy<0x00100000) {             /* underflow */
            y = x*x;
            if(y!=x) {          /* raise underflow flag */
                CYG_LIBM_HI(y) = hx; CYG_LIBM_LO(y) = lx;
                return y;
            }
        }
        CYG_LIBM_HI(x) = hx; CYG_LIBM_LO(x) = lx;
        return x;
}

#endif // ifdef CYGPKG_LIBM     

// EOF s_nextafter.c
