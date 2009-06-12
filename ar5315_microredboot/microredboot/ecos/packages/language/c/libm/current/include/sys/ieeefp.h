#ifndef CYGONCE_LIBM_SYS_IEEEFP_H
#define CYGONCE_LIBM_SYS_IEEEFP_H
//===========================================================================
//
//      ieeefp.h
//
//      Definitions specific to IEEE-754 floating-point format
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
// Description: Definitions specific to IEEE-754 floating-point format
// Usage:       #include <sys/ieeefp.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libm.h>   // Configuration header

// Include the Math library?
#ifdef CYGPKG_LIBM     

// INCLUDES

#include <cyg/infra/cyg_type.h>     // Common type definitions and support
                                    // including endian-ness

#if (CYG_BYTEORDER == CYG_MSBFIRST) // Big endian

// Note: there do not seem to be any current machines which are Big Endian but
// have a mixed up double layout. 

typedef union 
{
    cyg_int32 asi32[2];

    cyg_int64 asi64;
    
    double value;
    
    struct 
    {
        unsigned int sign : 1;
        unsigned int exponent: 11;
        unsigned int fraction0:4;
        unsigned int fraction1:16;
        unsigned int fraction2:16;
        unsigned int fraction3:16;
        
    } number;
    
    struct 
    {
        unsigned int sign : 1;
        unsigned int exponent: 11;
        unsigned int quiet:1;
        unsigned int function0:3;
        unsigned int function1:16;
        unsigned int function2:16;
        unsigned int function3:16;
    } nan;
    
    struct 
    {
        cyg_uint32 msw;
        cyg_uint32 lsw;
    } parts;

    
} Cyg_libm_ieee_double_shape_type;


typedef union
{
    cyg_int32 asi32;
    
    float value;

    struct 
    {
        unsigned int sign : 1;
        unsigned int exponent: 8;
        unsigned int fraction0: 7;
        unsigned int fraction1: 16;
    } number;

    struct 
    {
        unsigned int sign:1;
        unsigned int exponent:8;
        unsigned int quiet:1;
        unsigned int function0:6;
        unsigned int function1:16;
    } nan;
    
} Cyg_libm_ieee_float_shape_type;


#else // Little endian

typedef union 
{
    cyg_int32 asi32[2];

    cyg_int64 asi64;
    
    double value;

    struct 
    {
#if (CYG_DOUBLE_BYTEORDER == CYG_MSBFIRST) // Big endian
        unsigned int fraction1:16;
        unsigned int fraction0: 4;
        unsigned int exponent :11;
        unsigned int sign     : 1;
        unsigned int fraction3:16;
        unsigned int fraction2:16;
#else
        unsigned int fraction3:16;
        unsigned int fraction2:16;
        unsigned int fraction1:16;
        unsigned int fraction0: 4;
        unsigned int exponent :11;
        unsigned int sign     : 1;
#endif
    } number;

    struct 
    {
#if (CYG_DOUBLE_BYTEORDER == CYG_MSBFIRST) // Big endian
        unsigned int function1:16;
        unsigned int function0:3;
        unsigned int quiet:1;
        unsigned int exponent: 11;
        unsigned int sign : 1;
        unsigned int function3:16;
        unsigned int function2:16;
#else
        unsigned int function3:16;
        unsigned int function2:16;
        unsigned int function1:16;
        unsigned int function0:3;
        unsigned int quiet:1;
        unsigned int exponent: 11;
        unsigned int sign : 1;
#endif
    } nan;

    struct 
    {
#if (CYG_DOUBLE_BYTEORDER == CYG_MSBFIRST) // Big endian
        cyg_uint32 msw;
        cyg_uint32 lsw;
#else
        cyg_uint32 lsw;
        cyg_uint32 msw;
#endif
    } parts;
    
} Cyg_libm_ieee_double_shape_type;


typedef union
{
    cyg_int32 asi32;
  
    float value;

    struct 
    {
        unsigned int fraction0: 7;
        unsigned int fraction1: 16;
        unsigned int exponent: 8;
        unsigned int sign : 1;
    } number;

    struct 
    {
        unsigned int function1:16;
        unsigned int function0:6;
        unsigned int quiet:1;
        unsigned int exponent:8;
        unsigned int sign:1;
    } nan;

} Cyg_libm_ieee_float_shape_type;

#endif // little-endian


#endif // ifdef CYGPKG_LIBM     

#endif // CYGONCE_LIBM_SYS_IEEEFP_H multiple inclusion protection

// EOF ieeefp.h
