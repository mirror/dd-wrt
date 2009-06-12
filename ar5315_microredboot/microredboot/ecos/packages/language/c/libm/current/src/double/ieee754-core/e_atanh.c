//===========================================================================
//
//      e_atanh.c
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


/* @(#)e_atanh.c 1.3 95/01/18 */
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

/* __ieee754_atanh(x)
 * Method :
 *    1.Reduced x to positive by atanh(-x) = -atanh(x)
 *    2.For x>=0.5
 *                  1              2x                          x
 *      atanh(x) = --- * log(1 + -------) = 0.5 * log1p(2 * --------)
 *                  2             1 - x                      1 - x
 *      
 *      For x<0.5
 *      atanh(x) = 0.5*log1p(2x+2x*x/(1-x))
 *
 * Special cases:
 *      atanh(x) is NaN if |x| > 1 with signal;
 *      atanh(NaN) is that NaN with no signal;
 *      atanh(+-1) is +-INF with signal.
 *
 */

#include "mathincl/fdlibm.h"

static const double one = 1.0, huge = 1e300;

static double zero = 0.0;

        double __ieee754_atanh(double x)
{
        double t;
        int hx,ix;
        unsigned lx;
        hx = CYG_LIBM_HI(x);            /* high word */
        lx = CYG_LIBM_LO(x);            /* low word */
        ix = hx&0x7fffffff;
        if ((ix|((lx|(-lx))>>31))>0x3ff00000) /* |x|>1 */
            return (x-x)/(x-x);
        if(ix==0x3ff00000) 
            return x/zero;
        if(ix<0x3e300000&&(huge+x)>zero) return x;      /* x<2**-28 */
        CYG_LIBM_HI(x) = ix;            /* x <- |x| */
        if(ix<0x3fe00000) {             /* x < 0.5 */
            t = x+x;
            t = 0.5*log1p(t+t*x/(one-x));
        } else 
            t = 0.5*log1p((x+x)/(one-x));
        if(hx>=0) return t; else return -t;
}

#endif // ifdef CYGPKG_LIBM     

// EOF e_atanh.c
