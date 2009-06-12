//===========================================================================
//
//      w_scalb.c
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


/* @(#)w_scalb.c 1.3 95/01/18 */
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
 * wrapper scalb(double x, double fn) is provide for
 * passing various standard test suite. One 
 * should use scalbn() instead.
 */

#include "mathincl/fdlibm.h"

#ifndef CYGSEM_LIBM_COMPAT_IEEE_ONLY
# include <errno.h>
#endif

#ifdef CYGFUN_LIBM_SVID3_scalb
        double scalb(double x, double fn)       /* wrapper scalb */
#else
        double scalb(double x, int fn)          /* wrapper scalb */
#endif
{
#ifdef CYGSEM_LIBM_COMPAT_IEEE_ONLY
        return __ieee754_scalb(x,fn);
#else
        double z;
        z = __ieee754_scalb(x,fn);
        if(cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_IEEE) return z;
        if(!(finite(z)||isnan(z))&&finite(x)) {
            return __kernel_standard(x,(double)fn,32); /* scalb overflow */
        }
        if(z==0.0&&z!=x) {
            return __kernel_standard(x,(double)fn,33); /* scalb underflow */
        } 
#ifdef CYGFUN_LIBM_SVID3_scalb
        if(!finite(fn)) errno = ERANGE;
#endif
        return z;
#endif 
}

#endif // ifdef CYGPKG_LIBM     

// EOF w_scalb.c
