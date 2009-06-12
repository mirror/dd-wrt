#ifndef CYGONCE_LIBC_I18N_NEWLIBCTYPE_H
#define CYGONCE_LIBC_I18N_NEWLIBCTYPE_H
/*===========================================================================
//
//      newlibctype.h
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
// Purpose:     
// Description: 
// Usage:        Do not include this file directly - use #include <ctype.h>
//
//####DESCRIPTIONEND####
//
//=========================================================================*/

/* CONFIGURATION */

#include <pkgconf/libc_i18n.h>   /* Configuration header */

#ifdef __cplusplus
extern "C" {
#endif

extern int isalnum(int __c);
extern int isalpha(int __c);
extern int iscntrl(int __c);
extern int isdigit(int __c);
extern int isgraph(int __c);
extern int islower(int __c);
extern int isprint(int __c);
extern int ispunct(int __c);
extern int isspace(int __c);
extern int isupper(int __c);
extern int isxdigit(int __c);
extern int tolower(int __c);
extern int toupper(int __c);

#ifndef __STRICT_ANSI__
int isascii(int __c);
int toascii(int __c);
int _tolower(int __c);
int _toupper(int __c);
#endif

#define _U      01
#define _L      02
#define _N      04
#define _S      010
#define _P      020
#define _C      040
#define _X      0100
#define _B      0200

extern  const char    _ctype_[];

#ifndef __cplusplus
#define isalpha(c)      ((_ctype_+1)[(unsigned)(c)]&(_U|_L))
#define isupper(c)      ((_ctype_+1)[(unsigned)(c)]&_U)
#define islower(c)      ((_ctype_+1)[(unsigned)(c)]&_L)
#define isdigit(c)      ((_ctype_+1)[(unsigned)(c)]&_N)
#define isxdigit(c)     ((_ctype_+1)[(unsigned)(c)]&(_X|_N))
#define isspace(c)      ((_ctype_+1)[(unsigned)(c)]&_S)
#define ispunct(c)      ((_ctype_+1)[(unsigned)(c)]&_P)
#define isalnum(c)      ((_ctype_+1)[(unsigned)(c)]&(_U|_L|_N))
#define isprint(c)      ((_ctype_+1)[(unsigned)(c)]&(_P|_U|_L|_N|_B))
#define isgraph(c)      ((_ctype_+1)[(unsigned)(c)]&(_P|_U|_L|_N))
#define iscntrl(c)      ((_ctype_+1)[(unsigned)(c)]&_C)
/* Non-gcc versions will get the library versions, and will be
   slightly slower */
#ifdef __GNUC__
# define toupper(c) \
        __extension__ ({ int __x = (c); islower(__x) ? (__x - 'a' + 'A') : __x;})
# define tolower(c) \
        __extension__ ({ int __x = (c); isupper(__x) ? (__x - 'A' + 'a') : __x;})
#endif
#endif /* !__cplusplus */

#ifndef __STRICT_ANSI__
#define isascii(c)      ((unsigned)(c)<=0177)
#define toascii(c)      ((c)&0177)
#endif

#ifdef __cplusplus
}
#endif
#endif /* CYGONCE_LIBC_I18N_NEWLIBCTYPE_H multiple inclusion protection */

/* EOF newlibctype.h */
