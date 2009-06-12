#ifndef CYGONCE_ISO_LOCALE_H
#define CYGONCE_ISO_LOCALE_H
/*========================================================================
//
//      locale.h
//
//      ISO locale functions
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-04-14
// Purpose:       This file provides the locale functions required by 
//                ISO C and POSIX 1003.1.
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <locale.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/isoinfra.h>          /* Configuration header */

/* INCLUDES */

/* This is the "standard" way to get NULL from stddef.h,
 * which is the canonical location of the definitions.
 */
#define __need_NULL
#include <stddef.h>

#if CYGINT_ISO_LOCALE
# ifdef CYGBLD_ISO_LOCALE_HEADER
#  include CYGBLD_ISO_LOCALE_HEADER
# else

/* TYPE DEFINITIONS */

/* struct lconv contains information about numeric and monetary numbers
 * and is described in the ISO C standard section 7.4 */

struct lconv {

    /* the following affect formatted NONMONETARY QUANTITIES only */
    char *decimal_point;    /* decimal point                                */
    char *thousands_sep;    /* separates groups of digits before decimal
                               point                                        */
    char *grouping;         /* string whose elements indicate the size      */
                            /* of each group of digits                      */

    /* the following affect formatted MONETARY QUANTITIES only              */
    char *int_curr_symbol;   /* international curreny symbol                */
    char *currency_symbol;   /* local currency symbol                       */
    char *mon_decimal_point; /* decimal point                               */
    char *mon_thousands_sep; /* separator for groups of digits
                                before the decimal point                    */
    char *mon_grouping;      /* string whose elements indicate the size
                                of each group of digits                     */
    char *positive_sign;     /* string to indicate zero or positive value   */
    char *negative_sign;     /* string to indicate negative value           */
    char int_frac_digits;    /* number of digits after decimal point for
                                internationally formatted monetary nums     */
    char frac_digits;        /* number of digits after decimal point for
                                formatted monetary nums                     */
    char p_cs_precedes;      /* 1 if currency_symbol precedes non-negative
                                monetary quantity. 0 if succeeds            */
    char p_sep_by_space;     /* 1 if currency_symbol separated from value 
                                of non-negative monetary quantity by space.
                                Otherwise 0.                                */
    char n_cs_precedes;      /* 1 if currency_symbol precedes negative
                                monetary quantity. 0 if succeeds            */
    char n_sep_by_space;     /* 1 if currency_symbol separated from value
                                of negative monetary quantity by space.
                                Otherwise 0.                                */
    char p_sign_posn;        /* set according to position of positive_sign  */
    char n_sign_posn;        /* set according to position of negative_sign  */
};

/* CONSTANTS */

#define LC_COLLATE  (1<<0)
#define LC_CTYPE    (1<<1)
#define LC_MONETARY (1<<2)
#define LC_NUMERIC  (1<<3)
#define LC_TIME     (1<<4)
#define LC_ALL      (LC_COLLATE|LC_CTYPE|LC_MONETARY|LC_NUMERIC|LC_TIME)

/* FUNCTION PROTOTYPES */

#ifdef __cplusplus
extern "C" {
#endif

extern char *
setlocale( int /* category */, const char * /* locale */ );

extern struct lconv *
localeconv( void );

#ifdef __cplusplus
} /* extern "C" */
#endif 

# endif /* #elif !defined(CYGBLD_ISO_LOCALE_HEADER) */
#endif /* CYGINT_ISO_LOCALE */

#endif /* CYGONCE_ISO_LOCALE_H multiple inclusion protection */

/* EOF locale.h */
