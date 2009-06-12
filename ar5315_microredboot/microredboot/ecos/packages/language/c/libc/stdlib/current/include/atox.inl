#ifndef CYGONCE_LIBC_STDLIB_ATOX_INL
#define CYGONCE_LIBC_STDLIB_ATOX_INL
/*===========================================================================
//
//      atox.inl
//
//      Inline implementations for the ISO standard utility functions
//      atoi(), atol() and atof() defined in section 7.10 of the standard
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
// Author(s):    jlarmour
// Contributors: 
// Date:         2000-04-28
// Purpose:     
// Description: 
// Usage:        Do not include this file directly - include <stdlib.h> instead
//
//####DESCRIPTIONEND####
//
//=========================================================================*/

/* CONFIGURATION */

#include <pkgconf/libc_stdlib.h>    /* Configuration header */

/* INCLUDES */

#include <stddef.h>                 /* NULL */
#include <cyg/infra/cyg_trac.h>     /* Tracing support */

/* FUNCTION PROTOTYPES */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CYGFUN_LIBC_strtod
extern double
atof( const char * /* double_str */ );
#endif

extern int
atoi( const char * /* int_str */ );

extern long
atol( const char * /* long_str */ );

extern long long
atoll( const char * /* long_long_str */ );

#ifdef CYGFUN_LIBC_strtod
extern double
strtod( const char * /* double_str */, char ** /* endptr */ );
#endif

extern long
strtol( const char * /* long_str */, char ** /* endptr */,
        int /* base */ );

extern unsigned long
strtoul( const char * /* ulong_str */, char ** /* endptr */,
         int /* base */ );

#ifdef CYGFUN_LIBC_STDLIB_CONV_LONGLONG
extern long long
strtoll( const char * /* long_long_str */, char ** /* endptr */,
        int /* base */ );

extern unsigned long long
strtoull( const char * /* ulong_long_str */, char ** /* endptr */,
         int /* base */ );
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif 

/* INLINE FUNCTIONS */

/* 7.10.1 String conversion functions */

#ifndef CYGPRI_LIBC_STDLIB_ATOX_INLINE
# define CYGPRI_LIBC_STDLIB_ATOX_INLINE extern __inline__
#endif


#ifdef CYGFUN_LIBC_strtod
CYGPRI_LIBC_STDLIB_ATOX_INLINE double
atof( const char *__nptr )
{
    double __retval;

    CYG_REPORT_FUNCNAMETYPE( "atof", "returning %f" );

    CYG_CHECK_DATA_PTR( __nptr, "__nptr is an invalid pointer!" );
    
    __retval = strtod( __nptr, (char **)NULL );

    CYG_REPORT_RETVAL( __retval );

    return __retval;
} /* atof() */
#endif

CYGPRI_LIBC_STDLIB_ATOX_INLINE int
atoi( const char *__nptr )
{
    int __retval;

    CYG_REPORT_FUNCNAMETYPE( "atoi", "returning %d" );

    CYG_CHECK_DATA_PTR( __nptr, "__nptr is an invalid pointer!" );
    
    __retval = (int)strtol( __nptr, (char **)NULL, 10 );

    CYG_REPORT_RETVAL( __retval );

    return __retval;
} /* atoi() */


CYGPRI_LIBC_STDLIB_ATOX_INLINE long
atol( const char *__nptr )
{
    long __retval;

    CYG_REPORT_FUNCNAMETYPE( "atol", "returning %ld" );

    CYG_CHECK_DATA_PTR( __nptr, "__nptr is an invalid pointer!" );
    
    __retval = strtol( __nptr, (char **)NULL, 10 );

    CYG_REPORT_RETVAL( __retval );

    return __retval;
} /* atol() */

#ifdef CYGFUN_LIBC_STDLIB_CONV_LONGLONG
CYGPRI_LIBC_STDLIB_ATOX_INLINE long long
atoll( const char *__nptr )
{
    long long __retval;

    CYG_REPORT_FUNCNAMETYPE( "atoll", "returning %lld" );

    CYG_CHECK_DATA_PTR( __nptr, "__nptr is an invalid pointer!" );
    
    __retval = strtoll( __nptr, (char **)NULL, 10 );

    CYG_REPORT_RETVAL( __retval );

    return __retval;
} /* atoll() */
#endif

#endif /* CYGONCE_LIBC_STDLIB_ATOX_INL multiple inclusion protection */

/* EOF atox.inl */
