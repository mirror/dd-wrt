/*===========================================================================
//
//      newlibctype.cxx
//
//      newlib's implementation of the ctype functions
//
//===========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 eCosCentric Ltd.
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: 
// Date:         2004-02-16
// Purpose:      Provides newlib's implementation of the ctype functions.
// Description:  Also provides a degree of binary compatibility with newlib
//               ctype functions, primarily for the benefit of libstdc++.
// Usage:       
//
//####DESCRIPTIONEND####
//
//=========================================================================*/

/* CONFIGURATION */

#include <pkgconf/libc_i18n.h>   // Configuration header

#ifdef CYGPKG_LIBC_I18N_NEWLIB_CTYPE

#include <cyg/infra/cyg_type.h>
#include <cyg/libc/i18n/newlibctype.h>

#define _CONST  const    // just for compatibility

#define _CTYPE_DATA_0_127 \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C|_S,  _C|_S,  _C|_S,  _C|_S,  _C|_S,  _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _C,     _C,     _C,     _C,     _C,     _C,     _C,     _C, \
        _S|_B,  _P,     _P,     _P,     _P,     _P,     _P,     _P, \
        _P,     _P,     _P,     _P,     _P,     _P,     _P,     _P, \
        _N,     _N,     _N,     _N,     _N,     _N,     _N,     _N, \
        _N,     _N,     _P,     _P,     _P,     _P,     _P,     _P, \
        _P,     _U|_X,  _U|_X,  _U|_X,  _U|_X,  _U|_X,  _U|_X,  _U, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, \
        _U,     _U,     _U,     _U,     _U,     _U,     _U,     _U, \
        _U,     _U,     _U,     _P,     _P,     _P,     _P,     _P, \
        _P,     _L|_X,  _L|_X,  _L|_X,  _L|_X,  _L|_X,  _L|_X,  _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _L,     _L,     _L,     _L,     _L,     _L,     _L,     _L, \
        _L,     _L,     _L,     _P,     _P,     _P,     _P,     _C

#define _CTYPE_DATA_128_256 \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0, \
        0,      0,      0,      0,      0,      0,      0,      0

_CONST char _ctype_[1 + 256] = {
        0,
        _CTYPE_DATA_0_127,
        _CTYPE_DATA_128_256
};
_CONST char *__ctype_ptr = _ctype_ + 1;

#undef _tolower
#undef tolower
#undef _toupper
#undef toupper

int
_tolower(int c)
{
    return isupper(c) ? (c) - 'A' + 'a' : c;
}

int
tolower(int c)
{
    return isupper(c) ? (c) - 'A' + 'a' : c;
}

int
_toupper(int c)
{
    return islower(c) ? c - 'a' + 'A' : c;
}

int
toupper(int c)
{
    return islower(c) ? c - 'a' + 'A' : c;
}

#undef isalnum
#undef isalpha
#undef isascii
#undef iscntrl
#undef isdigit
#undef islower
#undef isgraph
#undef isprint
#undef ispunct
#undef isspace
#undef isupper
#undef isxdigit
#undef toascii

int
isalnum(int c)
{
    return((_ctype_ + 1)[c] & (_U|_L|_N));
}

int
isalpha(int c)
{
    return((_ctype_ + 1)[c] & (_U|_L));
}

int 
isascii(int c)
{
    return c >= 0 && c< 128;
}

int
iscntrl(int c)
{
    return((_ctype_ + 1)[c] & _C);
}

int
isdigit(int c)
{
    return((_ctype_ + 1)[c] & _N);
}

int
islower(int c)
{
    return((_ctype_ + 1)[c] & _L);
}

int
isgraph(int c)
{
    return((_ctype_ + 1)[c] & (_P|_U|_L|_N));
}

int
isprint(int c)
{
    return((_ctype_ + 1)[c] & (_P|_U|_L|_N|_B));
}

int
ispunct(int c)
{
    return((_ctype_ + 1)[c] & _P);
}

int
isspace(int c)
{
    return((_ctype_ + 1)[c] & _S);
}

int
isupper(int c)
{
    return((_ctype_ + 1)[c] & _U);
}

int
isxdigit(int c)
{
    return((_ctype_ + 1)[c] & ((_X)|(_N)));
}

int
toascii(int c)
{
    return (c)&0177;
}

#endif // ifdef CYGPKG_LIBC_I18N_NEWLIB_CTYPE

/* EOF newlibctype.cxx */
