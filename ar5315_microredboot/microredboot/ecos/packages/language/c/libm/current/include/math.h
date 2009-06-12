#ifndef CYGONCE_LIBM_MATH_H
#define CYGONCE_LIBM_MATH_H
//===========================================================================
//
//      math.h
//
//      Standard mathematical functions conforming to ANSI and other standards
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
// Description: Standard mathematical functions. These can be
//              configured to conform to ANSI section 7.5. There are also
//              a number of extensions conforming to IEEE-754 and behaviours
//              compatible with other standards
// Usage:       #include <math.h>
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
#include <float.h>                 // Properties of FP representation on this
                                   // platform
#include <sys/ieeefp.h>            // Cyg_libm_ieee_double_shape_type

// CONSTANT DEFINITIONS


// HUGE_VAL is a positive double (not necessarily representable as a float)
// representing infinity as specified in ANSI 7.5. cyg_libm_infinity is
// defined further down
#define HUGE_VAL        (cyg_libm_infinity.value)


#ifndef CYGSYM_LIBM_NO_XOPEN_SVID_NAMESPACE_POLLUTION
// HUGE is defined in System V Interface Definition 3 (SVID3) as the largest
// finite single precision number
#define HUGE            FLT_MAX    // from float.h


// Values used in the type field of struct exception below

#define DOMAIN          1
#define SING            2
#define OVERFLOW        3
#define UNDERFLOW       4
#define TLOSS           5
#define PLOSS           6


// TYPE DEFINITIONS

// Things required to support matherr() ( see comments in <pkgconf/libm.h>)

struct exception {
    int type;       // One of DOMAIN, SING, OVERFLOW, UNDERFLOW, TLOSS, PLOSS
    char *name;     // Name of the function generating the exception
    double arg1;    // First argument to the function
    double arg2;    // Second argument to the function
    double retval;  // Value to be returned - can be altered by matherr()
};

#endif // ifndef CYGSYM_LIBM_NO_XOPEN_SVID_NAMESPACE_POLLUTION


// GLOBALS

externC const Cyg_libm_ieee_double_shape_type cyg_libm_infinity;

//===========================================================================
// FUNCTION PROTOTYPES

// Functions not part of a standard

// This retrieves a pointer to the current compatibility mode of the Math
// library. See <pkgconf/libm.h> for the definition of Cyg_libm_compat_t

#ifdef CYGSEM_LIBM_THREAD_SAFE_COMPAT_MODE

externC Cyg_libm_compat_t
cyg_libm_get_compat_mode( void );

externC Cyg_libm_compat_t
cyg_libm_set_compat_mode( Cyg_libm_compat_t );

#else

externC Cyg_libm_compat_t cygvar_libm_compat_mode;

// Defined as static inline as it is unlikely that anyone wants to take the
// address of these functions.
//
// This returns the current compatibility mode

static inline Cyg_libm_compat_t
cyg_libm_get_compat_mode( void )
{
    return cygvar_libm_compat_mode;
}

// This sets the compatibility mode, and returns the previous mode
static inline Cyg_libm_compat_t
cyg_libm_set_compat_mode( Cyg_libm_compat_t math_compat_mode)
{
    Cyg_libm_compat_t oldmode;

    oldmode = cygvar_libm_compat_mode;
    cygvar_libm_compat_mode = math_compat_mode;
    return oldmode;
}

#endif // ifdef CYGSEM_LIBM_THREAD_SAFE_COMPAT_MODE

#ifdef CYGSEM_LIBM_THREAD_SAFE_GAMMA_FUNCTIONS

// FIXME: these need to be documented and signgam mentioned as non-ISO
// This returns the address of the signgam variable used by the gamma*() and
// lgamma*() functions
externC int *
cyg_libm_get_signgam_p( void );

#define signgam (*cyg_libm_get_signgam_p())

#else

externC int signgam;

#endif // ifdef CYGSEM_LIBM_THREAD_SAFE_GAMMA_FUNCTIONS

//===========================================================================
// Standard ANSI functions. Angles are always in radians

// Trigonometric functions - ANSI 7.5.2

externC double
acos( double );            // arc cosine i.e. inverse cos

externC double
asin( double );            // arc sine i.e. inverse sin

externC double
atan( double );            // arc tan i.e. inverse tan

externC double
atan2( double, double );   // arc tan of (first arg/second arg) using signs
                           // of args to determine quadrant

externC double
cos( double );             // cosine

externC double
sin( double );             // sine

externC double
tan( double );             // tangent

// Hyperbolic functions - ANSI 7.5.3

externC double
cosh( double );            // hyperbolic cosine

externC double
sinh( double );            // hyperbolic sine

externC double
tanh( double );            // hyperbolic tangent

// Exponential and Logarithmic Functions - ANSI 7.5.4

externC double
exp( double );             // exponent

externC double
frexp( double, int * );    // break number into normalized fraction (returned)
                           // and integral power of 2 (second arg)

externC double
ldexp( double, int );      // multiples number by integral power of 2 

externC double
log( double );             // natural logarithm

externC double
log10( double );           // base ten logarithm

externC double
modf( double, double * );  // break number into integral and fractional
                           // parts, each of which has same sign as arg.
                           // It returns signed fractional part, and
                           // puts integral part in second arg

// Power Functions - ANSI 7.5.5

externC double
pow( double, double );     // (1st arg) to the power of (2nd arg)

externC double
sqrt( double );            // square root

// Nearest integer, absolute value and remainder functions - ANSI 7.5.6

externC double
ceil( double );            // smallest integer >= arg

externC double
fabs( double );            // absolute value

externC double
floor( double );           // largest integer <= arg

externC double
fmod( double, double );    // remainder of (1st arg)/(2nd arg)

//===========================================================================
// Other standard functions

#ifndef CYGSYM_LIBM_NO_XOPEN_SVID_NAMESPACE_POLLUTION
externC int
matherr( struct exception * );    // User-overridable error handling - see
#endif                            // <pkgconf/libm.h> for a discussion

// FIXME: from here needs to be documented and mentioned as non-ISO
// Arc Hyperbolic trigonometric functions

externC double
acosh( double );                  // Arc hyperbolic cos i.e. inverse cosh

externC double
asinh( double );                  // Arc hyperbolic sin i.e. inverse sinh

externC double
atanh( double );                  // Arc hyperbolic tan i.e. inverse tanh

// Error functions

externC double                    // Error function, such that
erf( double );                    // erf(x) = 2/sqrt(pi) * integral from
                                  // 0 to x of e**(-t**2) dt

externC double                    // Complementary error function - simply
erfc( double );                   // 1.0 - erf(x)

// Gamma functions

externC double                    // Logarithm of the absolute value of the
lgamma( double );                 // gamma function of the argument. The
                                  // integer signgam is used to store the
                                  // sign of the gamma function of the arg

externC double
lgamma_r( double, int * );        // Re-entrant version of the above, where
                                  // the user passes the location of signgam
                                  // as the second argument

externC double                    // Identical to lgamma()!
gamma( double );                  // The reasons for this are historical,
                                  // and may be changed in future standards
                                  //
                                  // To get the real gamma function, you should
                                  // use: l=lgamma(x); g=signgam*exp(l);
                                  //
                                  // Do not just do signgam*exp(lgamma(x))
                                  // as lgamma() modifies signgam

externC double
gamma_r( double, int * );         // Identical to lgamma_r(). See above.


// Bessel functions

externC double                    // Zero-th order Bessel function of the
j0( double );                     // first kind at the ordinate of the argument

externC double                    // First-order Bessel function of the
j1( double );                     // first kind at the ordinate of the argument

externC double                    // Bessel function of the first kind of the
jn( int, double );                // order of the first argument at the
                                  // ordinate of the second argument

externC double                    // Zero-th order Bessel function of the
y0( double );                     // second kind at the ordinate of the
                                  // argument

externC double                    // First-order Bessel function of the
y1( double );                     // second kind at the ordinate of the
                                  // argument

externC double                    // Bessel function of the second kind of the
yn( int, double );                // order of the first argument at the
                                  // ordinate of the second argument

// scalb*()

externC double                    // scalbn(x,n) returns x*(2**n)
scalbn( double, int );

#ifdef CYGFUN_LIBM_SVID3_scalb

externC double
scalb( double, double );          // as above except n is a floating point arg

#else
externC double
scalb( double, int );             // as scalbn()

#endif // ifdef CYGFUN_LIBM_SVID3_scalb

// And the rest

externC double
cbrt( double );                   // Cube Root

externC double                    // hypotenuse function, defined such that:
hypot( double, double );          // hypot(x,y)==sqrt(x**2 + y**2)

externC int                       // whether the argument is NaN
isnan( double );

externC int
finite( double );                 // whether the argument is finite

externC double                    // logb returns the binary exponent of its
logb( double );                   // argument as an integral value
                                  // This is not recommended - use ilogb
                                  // instead

externC int                       // As for logb, but has the more correct
ilogb( double );                  // return value type of int


externC double                    // nextafter(x,y) returns the next
nextafter( double, double );      // representable floating point number
                                  // adjacent to x in the direction of y
                                  // i.e. the next greater FP if y>x, the next
                                  // less FP if y<x, or just x if y==x

externC double                    // remainder(x,y) returns the remainder
remainder( double, double );      // when x is divided by y

externC double                    // IEEE Test Vector
significand( double );            // significand(x) computes:
                                  //   scalb(x, (double) -ilogb(x))
                                 
//===========================================================================
// Non-standard functions

externC double                    // copysign(x,y) returns a number with
copysign ( double, double );      // the absolute value of x and the sign of y

externC double                    // rounds to an integer according to the
rint( double );                   // current rounding mode


// BSD functions

externC double                    // expm1(x) returns the equivalent of
expm1( double );                  // (exp(x) - 1) but more accurately when
                                  // x tends to zero

externC double                    // log1p(x) returns the equivalent of
log1p( double );                  // log(1+x) but more accurately when
                                  // x tends to zero

#endif // ifdef CYGPKG_LIBM     

#endif // CYGONCE_LIBM_MATH_H multiple inclusion protection

// EOF math.h
