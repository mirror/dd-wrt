//===========================================================================
//
//      e_fmod.c
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


/* @(#)e_fmod.c 1.3 95/01/18 */
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
 * __ieee754_fmod(x,y)
 * Return x mod y in exact arithmetic
 * Method: shift and subtract
 */

#include "mathincl/fdlibm.h"

static const double one = 1.0, Zero[] = {0.0, -0.0,};

        double __ieee754_fmod(double x, double y)
{
        int n,hx,hy,hz,ix,iy,sx,i;
        unsigned lx,ly,lz;

        hx = CYG_LIBM_HI(x);            /* high word of x */
        lx = CYG_LIBM_LO(x);            /* low  word of x */
        hy = CYG_LIBM_HI(y);            /* high word of y */
        ly = CYG_LIBM_LO(y);            /* low  word of y */
        sx = hx&0x80000000;             /* sign of x */
        hx ^=sx;                /* |x| */
        hy &= 0x7fffffff;       /* |y| */

    /* purge off exception values */
        if((hy|ly)==0||(hx>=0x7ff00000)||       /* y=0,or x not finite */
          ((hy|((ly|-ly)>>31))>0x7ff00000))     /* or y is NaN */
            return (x*y)/(x*y);
        if(hx<=hy) {
            if((hx<hy)||(lx<ly)) return x;      /* |x|<|y| return x */
            if(lx==ly) 
                return Zero[(unsigned)sx>>31];  /* |x|=|y| return x*0*/
        }

    /* determine ix = ilogb(x) */
        if(hx<0x00100000) {     /* subnormal x */
            if(hx==0) {
                for (ix = -1043, i=lx; i>0; i<<=1) ix -=1;
            } else {
                for (ix = -1022,i=(hx<<11); i>0; i<<=1) ix -=1;
            }
        } else ix = (hx>>20)-1023;

    /* determine iy = ilogb(y) */
        if(hy<0x00100000) {     /* subnormal y */
            if(hy==0) {
                for (iy = -1043, i=ly; i>0; i<<=1) iy -=1;
            } else {
                for (iy = -1022,i=(hy<<11); i>0; i<<=1) iy -=1;
            }
        } else iy = (hy>>20)-1023;

    /* set up {hx,lx}, {hy,ly} and align y to x */
        if(ix >= -1022) 
            hx = 0x00100000|(0x000fffff&hx);
        else {          /* subnormal x, shift x to normal */
            n = -1022-ix;
            if(n<=31) {
                hx = (hx<<n)|(lx>>(32-n));
                lx <<= n;
            } else {
                hx = lx<<(n-32);
                lx = 0;
            }
        }
        if(iy >= -1022) 
            hy = 0x00100000|(0x000fffff&hy);
        else {          /* subnormal y, shift y to normal */
            n = -1022-iy;
            if(n<=31) {
                hy = (hy<<n)|(ly>>(32-n));
                ly <<= n;
            } else {
                hy = ly<<(n-32);
                ly = 0;
            }
        }

    /* fix point fmod */
        n = ix - iy;
        while(n--) {
            hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
            if(hz<0){hx = hx+hx+(lx>>31); lx = lx+lx;}
            else {
                if((hz|lz)==0)          /* return sign(x)*0 */
                    return Zero[(unsigned)sx>>31];
                hx = hz+hz+(lz>>31); lx = lz+lz;
            }
        }
        hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
        if(hz>=0) {hx=hz;lx=lz;}

    /* convert back to floating value and restore the sign */
        if((hx|lx)==0)                  /* return sign(x)*0 */
            return Zero[(unsigned)sx>>31];      
        while(hx<0x00100000) {          /* normalize x */
            hx = hx+hx+(lx>>31); lx = lx+lx;
            iy -= 1;
        }
        if(iy>= -1022) {        /* normalize output */
            hx = ((hx-0x00100000)|((iy+1023)<<20));
            CYG_LIBM_HI(x) = hx|sx;
            CYG_LIBM_LO(x) = lx;
        } else {                /* subnormal output */
            n = -1022 - iy;
            if(n<=20) {
                lx = (lx>>n)|((unsigned)hx<<(32-n));
                hx >>= n;
            } else if (n<=31) {
                lx = (hx<<(32-n))|(lx>>n); hx = sx;
            } else {
                lx = hx>>(n-32); hx = sx;
            }
            CYG_LIBM_HI(x) = hx|sx;
            CYG_LIBM_LO(x) = lx;
            x *= one;           /* create necessary signal */
        }
        return x;               /* exact output */
}

#endif // ifdef CYGPKG_LIBM     

// EOF e_fmod.c
