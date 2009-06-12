//===========================================================================
//
//      e_remainder.c
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


/* @(#)e_remainder.c 1.3 95/01/18 */
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

/* __ieee754_remainder(x,p)
 * Return :                  
 *      returns  x REM p  =  x - [x/p]*p as if in infinite 
 *      precise arithmetic, where [x/p] is the (infinite bit) 
 *      integer nearest x/p (in half way case choose the even one).
 * Method : 
 *      Based on fmod() return x-[x/p]chopped*p exactlp.
 */

#include "mathincl/fdlibm.h"

static const double zero = 0.0;


        double __ieee754_remainder(double x, double p)
{
        int hx,hp;
        unsigned sx,lx,lp;
        double p_half;

        hx = CYG_LIBM_HI(x);            /* high word of x */
        lx = CYG_LIBM_LO(x);            /* low  word of x */
        hp = CYG_LIBM_HI(p);            /* high word of p */
        lp = CYG_LIBM_LO(p);            /* low  word of p */
        sx = hx&0x80000000;
        hp &= 0x7fffffff;
        hx &= 0x7fffffff;

    /* purge off exception values */
        if((hp|lp)==0) return (x*p)/(x*p);      /* p = 0 */
        if((hx>=0x7ff00000)||                   /* x not finite */
          ((hp>=0x7ff00000)&&                   /* p is NaN */
          (((hp-0x7ff00000)|lp)!=0)))
            return (x*p)/(x*p);


        if (hp<=0x7fdfffff) x = __ieee754_fmod(x,p+p);  /* now x < 2p */
        if (((hx-hp)|(lx-lp))==0) return zero*x;
        x  = fabs(x);
        p  = fabs(p);
        if (hp<0x00200000) {
            if(x+x>p) {
                x-=p;
                if(x+x>=p) x -= p;
            }
        } else {
            p_half = 0.5*p;
            if(x>p_half) {
                x-=p;
                if(x>=p_half) x -= p;
            }
        }
        CYG_LIBM_HI(x) ^= sx;
        return x;
}

#endif // ifdef CYGPKG_LIBM     

// EOF e_remainder.c
