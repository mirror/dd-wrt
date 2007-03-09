#ifndef CYGONCE_INFRA_CYG_TYPE_H
#define CYGONCE_INFRA_CYG_TYPE_H

//==========================================================================
//
//      cyg_type.h
//
//      Standard types, and some useful coding macros.
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg from an original by hmt
// Contributors:  nickg
// Date:        1997-09-08
// Purpose:     share unambiguously sized types.
// Description: we typedef [cyg_][u]int8,16,32 &c for general use.
// Usage:       #include "cyg/infra/cyg_type.h"
//              ...
//              cyg_int32 my_32bit_integer;
//              
//####DESCRIPTIONEND####
//

#include <stddef.h>           // Definition of NULL from the compiler

// -------------------------------------------------------------------------
// Some useful macros. These are defined here by default.

// __externC is used in mixed C/C++ headers to force C linkage on an external
// definition. It avoids having to put all sorts of ifdefs in.

#ifdef __cplusplus
# define __externC extern "C"
#else
# define __externC extern
#endif
// Also define externC for now - but it is deprecated
#define externC __externC

// -------------------------------------------------------------------------
// The header <basetype.h> defines the base types used here. It is
// supplied either by the target architecture HAL, or by the host
// porting kit. They are all defined as macros, and only those that
// make choices other than the defaults given below need be defined.

#define CYG_LSBFIRST 1234
#define CYG_MSBFIRST 4321

#include <cyg/hal/basetype.h>

#if (CYG_BYTEORDER != CYG_LSBFIRST) && (CYG_BYTEORDER != CYG_MSBFIRST)
# error You must define CYG_BYTEORDER to equal CYG_LSBFIRST or CYG_MSBFIRST
#endif

#ifndef CYG_DOUBLE_BYTEORDER
#define CYG_DOUBLE_BYTEORDER CYG_BYTEORDER
#endif

#ifndef cyg_halint8
# define cyg_halint8 char
#endif
#ifndef cyg_halint16
# define cyg_halint16 short
#endif
#ifndef cyg_halint32
# define cyg_halint32 int
#endif
#ifndef cyg_halint64
# define cyg_halint64 long long
#endif

#ifndef cyg_halcount8
# define cyg_halcount8 int
#endif
#ifndef cyg_halcount16
# define cyg_halcount16 int
#endif
#ifndef cyg_halcount32
# define cyg_halcount32 int
#endif
#ifndef cyg_halcount64
# define cyg_halcount64 long long
#endif

#ifndef cyg_haladdress
# define cyg_haladdress cyg_uint32
#endif
#ifndef cyg_haladdrword
# define cyg_haladdrword cyg_uint32
#endif

#ifndef cyg_halbool
# define cyg_halbool int
#endif

#ifndef cyg_halatomic
# define cyg_halatomic cyg_halint8
#endif

// -------------------------------------------------------------------------
// Provide a default architecture alignment
// This may be overridden in basetype.h if necessary.
// These should be straightforward numbers to allow use in assembly.

#ifndef CYGARC_ALIGNMENT
# define CYGARC_ALIGNMENT 8
#endif
// And corresponding power of two alignment
#ifndef CYGARC_P2ALIGNMENT
# define CYGARC_P2ALIGNMENT 3
#endif
#if (CYGARC_ALIGNMENT) != (1 << CYGARC_P2ALIGNMENT)
# error "Inconsistent CYGARC_ALIGNMENT and CYGARC_P2ALIGNMENT values"
#endif

// -------------------------------------------------------------------------
// The obvious few that compilers may define for you.
// But in case they don't:

#ifndef NULL
# define NULL 0
#endif

#ifndef __cplusplus

typedef cyg_halbool bool;

# ifndef false
#  define false 0
# endif

# ifndef true
#  define true (!false)
# endif

#endif

// -------------------------------------------------------------------------
// Allow creation of procedure-like macros that are a single statement,
// and must be followed by a semi-colon

#define CYG_MACRO_START do {
#define CYG_MACRO_END   } while (0)

#define CYG_EMPTY_STATEMENT CYG_MACRO_START CYG_MACRO_END

#define CYG_UNUSED_PARAM( _type_, _name_ ) CYG_MACRO_START      \
  _type_ __tmp1 = (_name_);                                     \
  _type_ __tmp2 = __tmp1;                                       \
  __tmp1 = __tmp2;                                              \
CYG_MACRO_END


// -------------------------------------------------------------------------
// Reference a symbol without explicitly making use of it. Ensures that
// the object containing the symbol will be included when linking.

#define CYG_REFERENCE_OBJECT(__object__)                                 \
     CYG_MACRO_START                                                     \
     static void *__cygvar_discard_me__ __attribute__ ((unused)) =       \
                                                          &(__object__); \
     CYG_MACRO_END

// -------------------------------------------------------------------------
// Define basic types for using integers in memory and structures;
// depends on compiler defaults and CPU type.

typedef unsigned cyg_halint8    cyg_uint8  ;
typedef   signed cyg_halint8    cyg_int8   ;

typedef unsigned cyg_halint16   cyg_uint16 ;
typedef   signed cyg_halint16   cyg_int16  ;

typedef unsigned cyg_halint32   cyg_uint32 ;
typedef   signed cyg_halint32   cyg_int32  ;

typedef unsigned cyg_halint64   cyg_uint64 ;
typedef   signed cyg_halint64   cyg_int64  ;

typedef  cyg_halbool            cyg_bool   ;

// -------------------------------------------------------------------------
// Define types for using integers in registers for looping and the like;
// depends on CPU type, choose what it is most comfortable with, with at
// least the range required.

typedef unsigned cyg_halcount8  cyg_ucount8  ;
typedef   signed cyg_halcount8  cyg_count8   ;

typedef unsigned cyg_halcount16 cyg_ucount16 ;
typedef   signed cyg_halcount16 cyg_count16  ;

typedef unsigned cyg_halcount32 cyg_ucount32 ;
typedef   signed cyg_halcount32 cyg_count32  ;

typedef unsigned cyg_halcount64 cyg_ucount64 ;
typedef   signed cyg_halcount64 cyg_count64  ;

// -------------------------------------------------------------------------
// Define a type to be used for atomic accesses. This type is guaranteed
// to be read or written in a single uninterruptible operation. This type
// is at least a single byte.

typedef volatile unsigned cyg_halatomic  cyg_atomic;
typedef volatile unsigned cyg_halatomic  CYG_ATOMIC;

// -------------------------------------------------------------------------
// Define types for access plain, on-the-metal memory or devices.

typedef cyg_uint32  CYG_WORD;
typedef cyg_uint8   CYG_BYTE;
typedef cyg_uint16  CYG_WORD16;
typedef cyg_uint32  CYG_WORD32;
typedef cyg_uint64  CYG_WORD64;

typedef cyg_haladdress  CYG_ADDRESS;
typedef cyg_haladdrword CYG_ADDRWORD;

// -------------------------------------------------------------------------
// Constructor ordering macros.  These are added as annotations to all
// static objects to order the constuctors appropriately.

#if defined(__cplusplus) && defined(__GNUC__) && \
    !defined(CYGBLD_ATTRIB_INIT_PRI)
# define CYGBLD_ATTRIB_INIT_PRI( _pri_ ) __attribute__((init_priority(_pri_)))
#elif !defined(CYGBLD_ATTRIB_INIT_PRI)
// FIXME: should maybe just bomb out if this is attempted anywhere else?
// Not sure
# define CYGBLD_ATTRIB_INIT_PRI( _pri_ )
#endif
    
// The following will be removed eventually as it doesn't allow the use of
// e.g. pri+5 format
#define CYG_INIT_PRIORITY( _pri_ ) CYGBLD_ATTRIB_INIT_PRI( CYG_INIT_##_pri_ )

#define CYGBLD_ATTRIB_INIT_BEFORE( _pri_ ) CYGBLD_ATTRIB_INIT_PRI(_pri_-100)
#define CYGBLD_ATTRIB_INIT_AFTER( _pri_ )  CYGBLD_ATTRIB_INIT_PRI(_pri_+100)

#define CYG_INIT_HAL                    10000
#define CYG_INIT_SCHEDULER              11000
#define CYG_INIT_INTERRUPTS             12000
#define CYG_INIT_DRIVERS                13000
#define CYG_INIT_CLOCK                  14000
#define CYG_INIT_IDLE_THREAD            15000
#define CYG_INIT_THREADS                16000
#define CYG_INIT_KERNEL                 40000
#define CYG_INIT_MEMALLOC               47000
#define CYG_INIT_IO                     49000
#define CYG_INIT_IO_FS                  50000
#define CYG_INIT_LIBC                   52000
#define CYG_INIT_COMPAT                 55000
#define CYG_INIT_APPLICATION            60000
#define CYG_INIT_PREDEFAULT             65534
#define CYG_INIT_DEFAULT                65535

// -------------------------------------------------------------------------
// Label name macros. Some toolsets generate labels with initial
// underscores and others don't. CYG_LABEL_NAME should be used on
// labels in C/C++ code that are defined in assembly code or linker
// scripts. CYG_LABEL_DEFN is for use in assembly code and linker
// scripts where we need to manufacture labels that can be used from
// C/C++.
// These are default implementations that should work for most targets.
// They may be overridden in basetype.h if necessary.

#ifndef CYG_LABEL_NAME

#define CYG_LABEL_NAME(_name_) _name_

#endif

#ifndef CYG_LABEL_DEFN

#define CYG_LABEL_DEFN(_label) _label

#endif

// -------------------------------------------------------------------------
// COMPILER-SPECIFIC STUFF

#ifdef __GNUC__
// Force a 'C' routine to be called like a 'C++' contructor
# if !defined(CYGBLD_ATTRIB_CONSTRUCTOR)
#  define CYGBLD_ATTRIB_CONSTRUCTOR __attribute__((constructor))
# endif

// Define a compiler-specific rune for saying a function doesn't return
# if !defined(CYGBLD_ATTRIB_NORET)
#  define CYGBLD_ATTRIB_NORET __attribute__((noreturn))
# endif

// How to define weak symbols - this is only relevant for ELF and a.out,
// but that won't be a problem for eCos
# if !defined(CYGBLD_ATTRIB_WEAK)
#  define CYGBLD_ATTRIB_WEAK __attribute__ ((weak))
# endif

// How to define alias to symbols. Just pass in the symbol itself, not
// the string name of the symbol
# if !defined(CYGBLD_ATTRIB_ALIAS)
#  define CYGBLD_ATTRIB_ALIAS(__symbol__) \
        __attribute__ ((alias (#__symbol__)))
# endif

// This effectively does the reverse of the previous macro. It defines
// a name that the attributed variable or function will actually have
// in assembler.
# if !defined(CYGBLD_ATTRIB_ASM_ALIAS)
#  define __Str(x) #x
#  define __Xstr(x) __Str(x)
#  define CYGBLD_ATTRIB_ASM_ALIAS(__symbol__) \
             __asm__ ( __Xstr( CYG_LABEL_DEFN( __symbol__ ) ) )
# endif

// Shows that a function returns the same value when given the same args, but
// note this can't be used if there are pointer args
# if !defined(CYGBLD_ATTRIB_CONST)
#  define CYGBLD_ATTRIB_CONST __attribute__((const))
#endif

// Assign a defined variable to a specific section
# if !defined(CYGBLD_ATTRIB_SECTION)
#  define CYGBLD_ATTRIB_SECTION(__sect__) __attribute__((section (__sect__)))
# endif

// Give a type or object explicit minimum alignment
# if !defined(CYGBLD_ATTRIB_ALIGN)
#  define CYGBLD_ATTRIB_ALIGN(__align__) __attribute__((aligned(__align__)))
# endif

# if !defined(CYGBLD_ATTRIB_ALIGN_MAX)
#  define CYGBLD_ATTRIB_ALIGN_MAX __attribute__((aligned))
# endif

# if !defined(CYGBLD_ATTRIB_ALIGNOFTYPE)
#  define CYGBLD_ATTRIB_ALIGNOFTYPE( _type_ ) \
     __attribute__((aligned(__alignof__( _type_ ))))
# endif

// Teach compiler how to check format of printf-like functions
# define CYGBLD_ATTRIB_PRINTF_FORMAT(__format__, __args__) \
        __attribute__((format (printf, __format__, __args__)))

#else // non-GNU

# define CYGBLD_ATTRIB_CONSTRUCTOR

# define CYGBLD_ATTRIB_NORET
    // This intentionally gives an error only if we actually try to
    // use it.  #error would give an error if we simply can't.
// FIXME: Had to disarm the bomb - the CYGBLD_ATTRIB_WEAK macro is now
//        (indirectly) used in host tools.
# define CYGBLD_ATTRIB_WEAK /* !!!-- Attribute weak not defined --!!! */

# define CYGBLD_ATTRIB_ALIAS(__x__) !!!-- Attribute alias not defined --!!!

# define CYGBLD_ATTRIB_ASM_ALIAS(__symbol__) !!!-- Asm alias not defined --!!!

# define CYGBLD_ATTRIB_CONST

# define CYGBLD_ATTRIB_ALIGN(__align__) !!!-- Alignment alias not defined --!!!

# define CYGBLD_ATTRIB_ALIGN_MAX !!!-- Alignment alias not defined --!!!

# define CYGBLD_ATTRIB_ALIGNOFTYPE( _type_ ) !!!-- Alignment alias not defined --!!!

# define CYGBLD_ATTRIB_PRINTF_FORMAT(__format__, __args__)

#endif

// How to define weak aliases. Currently this is simply a mixture of the
// above

# define CYGBLD_ATTRIB_WEAK_ALIAS(__symbol__) \
        CYGBLD_ATTRIB_WEAK CYGBLD_ATTRIB_ALIAS(__symbol__)

#ifdef __cplusplus
# define __THROW throw()
#else
# define __THROW
#endif

// -------------------------------------------------------------------------
// Variable annotations
// These annotations may be added to various static variables in the
// HAL and kernel to indicate which component they belong to. These
// are used by some targets to optimize memory placement of these
// variables.

#ifndef CYGBLD_ANNOTATE_VARIABLE_HAL
#define CYGBLD_ANNOTATE_VARIABLE_HAL
#endif
#ifndef CYGBLD_ANNOTATE_VARIABLE_SCHED
#define CYGBLD_ANNOTATE_VARIABLE_SCHED
#endif
#ifndef CYGBLD_ANNOTATE_VARIABLE_CLOCK
#define CYGBLD_ANNOTATE_VARIABLE_CLOCK
#endif
#ifndef CYGBLD_ANNOTATE_VARIABLE_INTR
#define CYGBLD_ANNOTATE_VARIABLE_INTR
#endif

// -------------------------------------------------------------------------
// Various "flavours" of memory regions that can be described by the 
// Memory Layout Tool (MLT).

#define CYGMEM_REGION_ATTR_R  0x01  // Region can be read
#define CYGMEM_REGION_ATTR_W  0x02  // Region can be written

// -------------------------------------------------------------------------
#endif // CYGONCE_INFRA_CYG_TYPE_H multiple inclusion protection
// EOF cyg_type.h
