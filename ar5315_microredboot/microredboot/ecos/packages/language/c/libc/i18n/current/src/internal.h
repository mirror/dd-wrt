#ifndef CYGONCE_LIBC_I18N_INTERNAL_H
#define CYGONCE_LIBC_I18N_INTERNAL_H

/*===========================================================================
//
//      internal.h
//
//      Internal header file to support the i18n locale functions.
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
// Author(s):    jjohnstn
// Contributors:
// Date:         2000-11-16
// Purpose:
// Description:
// Usage:        This file is used internally by i18n functions.
//
//####DESCRIPTIONEND####
//
//=========================================================================*/


/* escape character used for JIS encoding */
#define ESC_CHAR 0x1b

/* PROTOTYPES */

typedef int (*mbtowc_fn_type)(wchar_t *pwc, const char *s, size_t n, int *state);
typedef int (*wctomb_fn_type)(char *s, wchar_t wchar, int *state );

extern int __mbtowc_jp ( wchar_t *__pwc, const char *__s, size_t __n, int *__state );
extern int __mbtowc_c  ( wchar_t *__pwc, const char *__s, size_t __n, int *__state );
extern int __wctomb_jp ( char *__s, wchar_t __wchar, int *__state );

/* MACROS used to support SHIFT_JIS, EUC-JP, and JIS multibyte encodings */

#define _issjis1(c)    (((c) >= 0x81 && (c) <= 0x9f) || ((c) >= 0xe0 && (c) <= 0xef))
#define _issjis2(c)    (((c) >= 0x40 && (c) <= 0x7e) || ((c) >= 0x80 && (c) <= 0xfc))
#define _iseucjp(c)    ((c) >= 0xa1 && (c) <= 0xfe)
#define _isjis(c)      ((c) >= 0x21 && (c) <= 0x7e)

// TYPE DEFINITIONS

// define a type to encapsulate the locale. In time this will get much
// richer and complete

typedef struct {
  const char *name;
  struct lconv numdata;
  int mb_cur_max;
  /* if next two fields are NULL, it implies the default "C" single-byte handling. */
  mbtowc_fn_type mbtowc_fn;
  wctomb_fn_type wctomb_fn;
} Cyg_libc_locale_t;

// ctype locale pointer shared between setlocale and mb routines
extern const Cyg_libc_locale_t *__current_ctype_locale;

#endif /* CYGONCE_LIBC_I18N_INTERNAL_H */

