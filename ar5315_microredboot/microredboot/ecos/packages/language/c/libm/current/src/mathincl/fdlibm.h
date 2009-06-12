#ifndef CYGONCE_LIBM_MATHINCL_FDLIBM_H
#define CYGONCE_LIBM_MATHINCL_FDLIBM_H
//===========================================================================
//
//      fdlibm.h
//
//      Internal definitions for math library implementation based on fdlibm
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
// Author(s):     jlarmour
// Contributors:  jlarmour
// Date:          1999-02-09
// Purpose:     
// Description:   Internal implementation-specific header for math library
//                based on fdlibm
// Usage:         From within this package, #include "mathincl/fdlibm.h"
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libm.h>   // Configuration header

// Include the Math library?
#ifdef CYGPKG_LIBM     

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <math.h>                  // Main header for math library
#include <float.h>                 // Properties of FP representation on this
                                   // platform

// SANITY CHECKS

// Just check that we support IEEE-style 64-bit doubles. If not, this
// math library will not work
// This check will go away when support for single-precision alternatives are
// provided

#if DBL_MAX_EXP != 1024
# error IEEE-style 64-bit doubles are required to use the math library
#endif // if DBL_MAX_EXP == 1024


// TYPES

typedef cyg_int32   __int32_t;
typedef cyg_uint32  __uint32_t;
typedef Cyg_libm_ieee_double_shape_type ieee_double_shape_type;

// MACRO DEFINITIONS

#ifndef __STDC__
# define __STDC__ 1
#endif
#define CYG_LIBM_HI(__x)  (((Cyg_libm_ieee_double_shape_type *)&__x)->parts.msw)
#define CYG_LIBM_LO(__x)  (((Cyg_libm_ieee_double_shape_type *)&__x)->parts.lsw)
#define CYG_LIBM_HIp(__x) (((Cyg_libm_ieee_double_shape_type *)__x)->parts.msw)
#define CYG_LIBM_LOp(__x) (((Cyg_libm_ieee_double_shape_type *)__x)->parts.lsw)



/* Get two 32 bit ints from a double.  */

#define EXTRACT_WORDS(ix0,ix1,d)                                \
do {                                                            \
  Cyg_libm_ieee_double_shape_type ew_u;                         \
  ew_u.value = (d);                                             \
  (ix0) = ew_u.parts.msw;                                       \
  (ix1) = ew_u.parts.lsw;                                       \
} while (0)

/* Get the more significant 32 bit int from a double.  */

#define GET_HIGH_WORD(i,d)                                      \
do {                                                            \
  Cyg_libm_ieee_double_shape_type gh_u;                         \
  gh_u.value = (d);                                             \
  (i) = gh_u.parts.msw;                                         \
} while (0)

/* Get the less significant 32 bit int from a double.  */

#define GET_LOW_WORD(i,d)                                       \
do {                                                            \
  Cyg_libm_ieee_double_shape_type gl_u;                         \
  gl_u.value = (d);                                             \
  (i) = gl_u.parts.lsw;                                         \
} while (0)

/* Set a double from two 32 bit ints.  */

#define INSERT_WORDS(d,ix0,ix1)                                 \
do {                                                            \
  Cyg_libm_ieee_double_shape_type iw_u;                         \
  iw_u.parts.msw = (ix0);                                       \
  iw_u.parts.lsw = (ix1);                                       \
  (d) = iw_u.value;                                             \
} while (0)

/* Set the more significant 32 bits of a double from an int.  */

#define SET_HIGH_WORD(d,v)                                      \
do {                                                            \
  Cyg_libm_ieee_double_shape_type sh_u;                         \
  sh_u.value = (d);                                             \
  sh_u.parts.msw = (v);                                         \
  (d) = sh_u.value;                                             \
} while (0)

/* Set the less significant 32 bits of a double from an int.  */

#define SET_LOW_WORD(d,v)                                       \
do {                                                            \
  Cyg_libm_ieee_double_shape_type sl_u;                         \
  sl_u.value = (d);                                             \
  sl_u.parts.lsw = (v);                                         \
  (d) = sl_u.value;                                             \
} while (0)


// REPLACEMENTS FOR STUFF FROM MATH.H DUE TO CONFIG OPTION

#ifdef CYGSYM_LIBM_NO_XOPEN_SVID_NAMESPACE_POLLUTION

#define HUGE            FLT_MAX    // from float.h
#define DOMAIN          1
#define SING            2
#define OVERFLOW        3
#define UNDERFLOW       4
#define TLOSS           5
#define PLOSS           6

struct exception {
    int type;       // One of DOMAIN, SING, OVERFLOW, UNDERFLOW, TLOSS, PLOSS
    char *name;     // Name of the function generating the exception
    double arg1;    // First argument to the function
    double arg2;    // Second argument to the function
    double retval;  // Value to be returned - can be altered by matherr()
};

externC int
matherr( struct exception * );    // User-overridable error handling - see
                                  // <pkgconf/libm.h> for a discussion
#endif // ifdef CYGSYM_LIBM_NO_XOPEN_SVID_NAMESPACE_POLLUTION


// FUNCTION PROTOTYPES

// IEEE-754 style elementary functions */

externC double
__ieee754_sqrt( double );

externC double
__ieee754_acos( double );

externC double
__ieee754_acosh( double );

externC double
__ieee754_log( double );

externC double
__ieee754_atanh( double );

externC double
__ieee754_asin( double );

externC double
__ieee754_atan2( double, double );

externC double
__ieee754_exp( double );

externC double
__ieee754_cosh( double );

externC double
__ieee754_fmod( double, double );

externC double
__ieee754_pow( double, double );

externC double
__ieee754_lgamma_r( double, int * );

externC double
__ieee754_gamma_r( double, int * );

externC double
__ieee754_lgamma( double );

externC double
__ieee754_gamma( double );

externC double
__ieee754_log10( double );

externC double
__ieee754_sinh( double );

externC double
__ieee754_hypot( double, double );

externC double
__ieee754_j0( double );

externC double
__ieee754_j1( double );

externC double
__ieee754_y0( double );

externC double
__ieee754_y1( double );

externC double
__ieee754_jn( int, double );

externC double
__ieee754_yn( int, double );

externC double
__ieee754_remainder( double, double );

externC int
__ieee754_rem_pio2( double, double * );

#ifdef CYGFUN_LIBM_SVID3_scalb
externC double
__ieee754_scalb( double, double );
#else
externC double
__ieee754_scalb( double, int );
#endif

// FDLIBM kernel functions

externC double
__kernel_standard( double, double, int );

externC double
__kernel_sin( double, double, int );

externC double
__kernel_cos( double, double );

externC double
__kernel_tan( double, double, int );

externC int
__kernel_rem_pio2( double *, double *, int, int, int, const int * );

#endif // ifdef CYGPKG_LIBM     

#endif // CYGONCE_LIBM_MATHINCL_FDLIBM_H multiple inclusion protection

// EOF fdlibm.h
