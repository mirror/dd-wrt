//===========================================================================
//
//      signgam.cxx
//
//      Support sign of the gamma*() functions in Math library
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
// Description: Contains the accessor functions to get and set the stored sign
//              of the gamma*() functions in the math library
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

/* @(#)k_standard.c 1.3 95/01/18 */
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

// CONFIGURATION

#include <pkgconf/libm.h>   // Configuration header

// Include the Math library?
#ifdef CYGPKG_LIBM

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_trac.h>    // Tracing macros

#include <math.h>                  // Main header for math library
#include "mathincl/fdlibm.h"       // Internal header for math library

#include <cyg/error/codes.h>       // standard error codes

#ifndef CYGSEM_LIBM_COMPAT_IEEE_ONLY
#include <errno.h>
#else
static int errno; // this whole file won't be used if we're IEEE only, but
                  // doing this keeps the compiler happy
#endif

#ifdef CYGSEM_LIBM_USE_STDERR

#include <stdio.h>
#define WRITE2(u,v)     fputs(u, stderr)

#else

#define WRITE2(u,v)     0

#endif // ifdef CYGSEM_LIBM_USE_STDERR


// GLOBALS

static const double zero = 0.0;

// FUNCTIONS

/* 
 * Standard conformance (non-IEEE) on exception cases.
 * Mapping:
 *      1 -- acos(|x|>1)
 *      2 -- asin(|x|>1)
 *      3 -- atan2(+-0,+-0)
 *      4 -- hypot overflow
 *      5 -- cosh overflow
 *      6 -- exp overflow
 *      7 -- exp underflow
 *      8 -- y0(0)
 *      9 -- y0(-ve)
 *      10-- y1(0)
 *      11-- y1(-ve)
 *      12-- yn(0)
 *      13-- yn(-ve)
 *      14-- lgamma(finite) overflow
 *      15-- lgamma(-integer)
 *      16-- log(0)
 *      17-- log(x<0)
 *      18-- log10(0)
 *      19-- log10(x<0)
 *      20-- pow(0.0,0.0)
 *      21-- pow(x,y) overflow
 *      22-- pow(x,y) underflow
 *      23-- pow(0,negative) 
 *      24-- pow(neg,non-integral)
 *      25-- sinh(finite) overflow
 *      26-- sqrt(negative)
 *      27-- fmod(x,0)
 *      28-- remainder(x,0)
 *      29-- acosh(x<1)
 *      30-- atanh(|x|>1)
 *      31-- atanh(|x|=1)
 *      32-- scalb overflow
 *      33-- scalb underflow
 *      34-- j0(|x|>X_TLOSS)
 *      35-- y0(x>X_TLOSS)
 *      36-- j1(|x|>X_TLOSS)
 *      37-- y1(x>X_TLOSS)
 *      38-- jn(|x|>X_TLOSS, n)
 *      39-- yn(x>X_TLOSS, n)
 *      40-- gamma(finite) overflow
 *      41-- gamma(-integer)
 *      42-- pow(NaN,0.0)
 *      43-- ldexp overflow
 *      44-- ldexp underflow
 */


double
__kernel_standard(double x, double y, int type) 
{
        struct exception exc;

#ifdef CYGSEM_LIBM_USE_STDERR
        (void) fflush(stdout);
#endif
        exc.arg1 = x;
        exc.arg2 = y;
        switch(type) {
            case 1:
                /* acos(|x|>1) */
                exc.type = DOMAIN;
                exc.name = "acos";
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if(cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                    (void) WRITE2("acos: DOMAIN error\n", 19);
                  }
                  errno = EDOM;
                }
                break;
            case 2:
                /* asin(|x|>1) */
                exc.type = DOMAIN;
                exc.name = "asin";
                exc.retval = zero;
                if(cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if(cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("asin: DOMAIN error\n", 19);
                  }
                  errno = EDOM;
                }
                break;
            case 3:
                /* atan2(+-0,+-0) */
                exc.arg1 = y;
                exc.arg2 = x;
                exc.type = DOMAIN;
                exc.name = "atan2";
                exc.retval = zero;
                if(cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if(cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("atan2: DOMAIN error\n", 20);
                      }
                  errno = EDOM;
                }
                break;
            case 4:
                /* hypot(finite,finite) overflow */
                exc.type = OVERFLOW;
                exc.name = "hypot";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = HUGE;
                else
                  exc.retval = HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!matherr(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 5:
                /* cosh(finite) overflow */
                exc.type = OVERFLOW;
                exc.name = "cosh";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = HUGE;
                else
                  exc.retval = HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!matherr(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 6:
                /* exp(finite) overflow */
                exc.type = OVERFLOW;
                exc.name = "exp";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = HUGE;
                else
                  exc.retval = HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!matherr(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 7:
                /* exp(finite) underflow */
                exc.type = UNDERFLOW;
                exc.name = "exp";
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!matherr(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 8:
                /* y0(0) = -inf */
                exc.type = DOMAIN;      /* should be SING for IEEE */
                exc.name = "y0";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("y0: DOMAIN error\n", 17);
                      }
                  errno = EDOM;
                }
                break;
            case 9:
                /* y0(x<0) = NaN */
                exc.type = DOMAIN;
                exc.name = "y0";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("y0: DOMAIN error\n", 17);
                      }
                  errno = EDOM;
                }
                break;
            case 10:
                /* y1(0) = -inf */
                exc.type = DOMAIN;      /* should be SING for IEEE */
                exc.name = "y1";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("y1: DOMAIN error\n", 17);
                      }
                  errno = EDOM;
                }
                break;
            case 11:
                /* y1(x<0) = NaN */
                exc.type = DOMAIN;
                exc.name = "y1";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("y1: DOMAIN error\n", 17);
                      }
                  errno = EDOM;
                }
                break;
            case 12:
                /* yn(n,0) = -inf */
                exc.type = DOMAIN;      /* should be SING for IEEE */
                exc.name = "yn";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("yn: DOMAIN error\n", 17);
                      }
                  errno = EDOM;
                }
                break;
            case 13:
                /* yn(x<0) = NaN */
                exc.type = DOMAIN;
                exc.name = "yn";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("yn: DOMAIN error\n", 17);
                      }
                  errno = EDOM;
                }
                break;
            case 14:
                /* lgamma(finite) overflow */
                exc.type = OVERFLOW;
                exc.name = "lgamma";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = HUGE;
                else
                  exc.retval = HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                        errno = ERANGE;
                else if (!matherr(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 15:
                /* lgamma(-integer) or lgamma(0) */
                exc.type = SING;
                exc.name = "lgamma";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = HUGE;
                else
                  exc.retval = HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("lgamma: SING error\n", 19);
                      }
                  errno = EDOM;
                }
                break;
            case 16:
                /* log(0) */
                exc.type = SING;
                exc.name = "log";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("log: SING error\n", 16);
                      }
                  errno = EDOM;
                }
                break;
            case 17:
                /* log(x<0) */
                exc.type = DOMAIN;
                exc.name = "log";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("log: DOMAIN error\n", 18);
                      }
                  errno = EDOM;
                }
                break;
            case 18:
                /* log10(0) */
                exc.type = SING;
                exc.name = "log10";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("log10: SING error\n", 18);
                      }
                  errno = EDOM;
                }
                break;
            case 19:
                /* log10(x<0) */
                exc.type = DOMAIN;
                exc.name = "log10";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("log10: DOMAIN error\n", 20);
                      }
                  errno = EDOM;
                }
                break;
            case 20:
                /* pow(0.0,0.0) */
                /* error only if cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID */
                exc.type = DOMAIN;
                exc.name = "pow";
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() != CYGNUM_LIBM_COMPAT_SVID) exc.retval = 1.0;
                else if (!matherr(&exc)) {
                        (void) WRITE2("pow(0,0): DOMAIN error\n", 23);
                        errno = EDOM;
                }
                break;
            case 21:
                /* pow(x,y) overflow */
                exc.type = OVERFLOW;
                exc.name = "pow";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                  exc.retval = HUGE;
                  y *= 0.5;
                  if(x<zero&&rint(y)!=y) exc.retval = -HUGE;
                } else {
                  exc.retval = HUGE_VAL;
                  y *= 0.5;
                  if(x<zero&&rint(y)!=y) exc.retval = -HUGE_VAL;
                }
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!matherr(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 22:
                /* pow(x,y) underflow */
                exc.type = UNDERFLOW;
                exc.name = "pow";
                exc.retval =  zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!matherr(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 23:
                /* 0**neg */
                exc.type = DOMAIN;
                exc.name = "pow";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) 
                  exc.retval = zero;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("pow(0,neg): DOMAIN error\n", 25);
                      }
                  errno = EDOM;
                }
                break;
            case 24:
                /* neg**non-integral */
                exc.type = DOMAIN;
                exc.name = "pow";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) 
                    exc.retval = zero;
                else 
                    exc.retval = zero/zero;     /* X/Open allow NaN */
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX) 
                   errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("neg**non-integral: DOMAIN error\n", 32);
                      }
                  errno = EDOM;
                }
                break;
            case 25:
                /* sinh(finite) overflow */
                exc.type = OVERFLOW;
                exc.name = "sinh";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = ( (x>zero) ? HUGE : -HUGE);
                else
                  exc.retval = ( (x>zero) ? HUGE_VAL : -HUGE_VAL);
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!matherr(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 26:
                /* sqrt(x<0) */
                exc.type = DOMAIN;
                exc.name = "sqrt";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = zero;
                else
                  exc.retval = zero/zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("sqrt: DOMAIN error\n", 19);
                      }
                  errno = EDOM;
                }
                break;
            case 27:
                /* fmod(x,0) */
                exc.type = DOMAIN;
                exc.name = "fmod";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                    exc.retval = x;
                else
                    exc.retval = zero/zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                    (void) WRITE2("fmod:  DOMAIN error\n", 20);
                  }
                  errno = EDOM;
                }
                break;
            case 28:
                /* remainder(x,0) */
                exc.type = DOMAIN;
                exc.name = "remainder";
                exc.retval = zero/zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                    (void) WRITE2("remainder: DOMAIN error\n", 24);
                  }
                  errno = EDOM;
                }
                break;
            case 29:
                /* acosh(x<1) */
                exc.type = DOMAIN;
                exc.name = "acosh";
                exc.retval = zero/zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                    (void) WRITE2("acosh: DOMAIN error\n", 20);
                  }
                  errno = EDOM;
                }
                break;
            case 30:
                /* atanh(|x|>1) */
                exc.type = DOMAIN;
                exc.name = "atanh";
                exc.retval = zero/zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                    (void) WRITE2("atanh: DOMAIN error\n", 20);
                  }
                  errno = EDOM;
                }
                break;
            case 31:
                /* atanh(|x|=1) */
                exc.type = SING;
                exc.name = "atanh";
                exc.retval = x/zero;    /* sign(x)*inf */
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                    (void) WRITE2("atanh: SING error\n", 18);
                  }
                  errno = EDOM;
                }
                break;
            case 32:
                /* scalb overflow; SVID also returns +-HUGE_VAL */
                exc.type = OVERFLOW;
                exc.name = "scalb";
                exc.retval = x > zero ? HUGE_VAL : -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!matherr(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 33:
                /* scalb underflow */
                exc.type = UNDERFLOW;
                exc.name = "scalb";
                exc.retval = copysign(zero,x);
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!matherr(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 34:
                /* j0(|x|>X_TLOSS) */
                exc.type = TLOSS;
                exc.name = "j0";
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                        errno = ERANGE;
                else if (!matherr(&exc)) {
                        if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                                (void) WRITE2(exc.name, 2);
                                (void) WRITE2(": TLOSS error\n", 14);
                        }
                        errno = ERANGE;
                }        
                break;
            case 35:
                /* y0(x>X_TLOSS) */
                exc.type = TLOSS;
                exc.name = "y0";
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                        errno = ERANGE;
                else if (!matherr(&exc)) {
                        if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                                (void) WRITE2(exc.name, 2);
                                (void) WRITE2(": TLOSS error\n", 14);
                        }
                        errno = ERANGE;
                }        
                break;
            case 36:
                /* j1(|x|>X_TLOSS) */
                exc.type = TLOSS;
                exc.name = "j1";
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                        errno = ERANGE;
                else if (!matherr(&exc)) {
                        if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                                (void) WRITE2(exc.name, 2);
                                (void) WRITE2(": TLOSS error\n", 14);
                        }
                        errno = ERANGE;
                }        
                break;
            case 37:
                /* y1(x>X_TLOSS) */
                exc.type = TLOSS;
                exc.name = "y1";
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                        errno = ERANGE;
                else if (!matherr(&exc)) {
                        if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                                (void) WRITE2(exc.name, 2);
                                (void) WRITE2(": TLOSS error\n", 14);
                        }
                        errno = ERANGE;
                }        
                break;
            case 38:
                /* jn(|x|>X_TLOSS) */
                exc.type = TLOSS;
                exc.name = "jn";
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                        errno = ERANGE;
                else if (!matherr(&exc)) {
                        if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                                (void) WRITE2(exc.name, 2);
                                (void) WRITE2(": TLOSS error\n", 14);
                        }
                        errno = ERANGE;
                }        
                break;
            case 39:
                /* yn(x>X_TLOSS) */
                exc.type = TLOSS;
                exc.name = "yn";
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                        errno = ERANGE;
                else if (!matherr(&exc)) {
                        if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                                (void) WRITE2(exc.name, 2);
                                (void) WRITE2(": TLOSS error\n", 14);
                        }
                        errno = ERANGE;
                }        
                break;
            case 40:
                /* gamma(finite) overflow */
                exc.type = OVERFLOW;
                exc.name = "gamma";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = HUGE;
                else
                  exc.retval = HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!matherr(&exc)) {
                  errno = ERANGE;
                }
                break;
            case 41:
                /* gamma(-integer) or gamma(0) */
                exc.type = SING;
                exc.name = "gamma";
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = HUGE;
                else
                  exc.retval = HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!matherr(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("gamma: SING error\n", 18);
                      }
                  errno = EDOM;
                }
                break;
            case 42:
                /* pow(NaN,0.0) */
                /* error only if cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID & CYGNUM_LIBM_COMPAT_XOPEN */
                exc.type = DOMAIN;
                exc.name = "pow";
                exc.retval = x;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_IEEE ||
                    cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX) exc.retval = 1.0;
                else if (!matherr(&exc)) {
                        errno = EDOM;
                }
                break;
            case 43:
                /* ldexp overflow; SVID also returns +-HUGE_VAL */
                exc.type = OVERFLOW;
                exc.name = "ldexp";
                exc.retval = x > zero ? HUGE_VAL : -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!matherr(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 44:
                /* ldexp underflow */
                exc.type = UNDERFLOW;
                exc.name = "ldexp";
                exc.retval = copysign(zero,x);
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!matherr(&exc)) {
                        errno = ERANGE;
                }
                break;
        }
        return exc.retval; 
}

#endif // ifdef CYGPKG_LIBM

// EOF standard.c
