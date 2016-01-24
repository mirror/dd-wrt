/*
 *  Copyright (C) 1999, 2000, 2001, 2002, 2003  James Antill
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  email: james@and.org
*/
/* Floating point output for `printf'.
   Copyright (C) 1995-1999, 2000, 2001, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */
/* This code does the %F, %f, %G, %g, %A, %a, %E, %e from *printf by using
 * the code from glibc */
/* NOTE: This file assumes the local char set is ASCII */
/* NOTE: This file assumes stuff about the double implementation */
/* NOTE: This file assumes long long exists */
/* NOTE: This file assumes alloca() exists */
/* NOTE: This file includes assembler, works for ia32 atm. */
/* Note that this file is #include'd */


#include "glibc/include/endian.h"

#include "glibc/sysdeps/ieee754/ieee754.h"

/* Pretend the world's a (FILE *) brain damage for glibc */
struct vstr__FILE_wrap
{
 Vstr_base *base;
 size_t pos_diff;
 struct Vstr__fmt_spec *spec;
};

struct vstr__fmt_printf_info
{
  int prec;                     /* Precision.  */
  int width;                    /* Width.  */
  wchar_t spec;                 /* Format letter.  */
  unsigned int is_long_double:1;/* L flag.  */
 /* unsigned int is_short:1; */      /* h flag.  */
 /* unsigned int is_long:1; */       /* l flag.  */
  unsigned int alt:1;           /* # flag.  */
  unsigned int space:1;         /* Space flag.  */
  unsigned int left:1;          /* - flag.  */
  unsigned int showsign:1;      /* + flag.  */
  unsigned int group:1;         /* ' flag.  */
#if 0
 unsigned int extra:1;         /* For special use.  */
 /* unsigned int is_char:1; */       /* hh flag.  */
 unsigned int wide:1;           /* Nonzero for wide character streams.  */
#endif
 /* unsigned int i18n:1; */          /* I flag.  */
  wchar_t pad;                  /* Padding character.  */
};

#ifndef CHAR_MAX /* for compiling against dietlibc */
# define CHAR_MAX SCHAR_MAX
#endif

#undef FILE
#define FILE struct vstr__FILE_wrap

#undef putc
#define putc(c, f) \
 (vstr_add_rep_chr((f)->base, (f)->base->len - (f)->pos_diff, c, 1) ? \
  (c) : EOF)

#define PUT(f, s, n) \
 vstr_add_buf((f)->base, ((f)->base->len - (f)->pos_diff), s, n)

#define PAD(f, c, n) \
 (vstr_add_rep_chr((f)->base, (f)->base->len - (f)->pos_diff, c, n) * n)

#undef   isupper
#define  isupper(x) (fp->spec->flags & LARGE)
#undef   tolower
#define  tolower(x) (VSTR__IS_ASCII_UPPER(x) ? VSTR__TO_ASCII_LOWER(x) : (x))
#undef  _tolower
#define _tolower(x) (VSTR__IS_ASCII_UPPER(x) ? VSTR__TO_ASCII_LOWER(x) : (x))


#ifndef HAVE_INLINE
# undef inline
# define inline /* nothing */
#endif

/* We use the GNU MP library to handle large numbers.

   An MP variable occupies a varying number of entries in its array.  We keep
   track of this number for efficiency reasons.  Otherwise we would always
   have to process the whole array.  */
#define MPN_VAR(name) mp_limb_t *name; mp_size_t name##size

#define MPN_ASSIGN(dst,src)                                                   \
  memcpy (dst, src, (dst##size = src##size) * sizeof (mp_limb_t))
#define MPN_GE(u,v) \
  (u##size > v##size || (u##size == v##size && __mpn_cmp (u, v, u##size) >= 0))

#if !defined(VSTR__LDOUBLE_BITS_128) && !defined(VSTR__LDOUBLE_BITS_96) && \
    defined(__i386__)
# define VSTR__LDOUBLE_BITS_96 1
#else
# error "No long double size defined"
#endif

#include "glibc/math/math_private.h"

#define mpn_add_1 vstr__fmt_dbl_mpn_add_1
#include "glibc/stdlib/gmp.h"

#include "glibc/stdlib/gmp-impl.h"

/* can have different byte order for floats */
#ifndef __FLOAT_WORD_ORDER
# define __FLOAT_WORD_ORDER BYTE_ORDER
#endif

#include "glibc/stdlib/longlong.h"

#ifndef add_ssaaaa
# error "No arch specific code for add_ssaaaa etc."
#endif

#include <float.h> /* from compiler ... LDBL_MIN_EXP FLT_MANT_DIG */

/* 128 bit long double */
#ifdef VSTR__LDOUBLE_BITS_128
# error "ldbl 128 bit"
#endif

/* 96 bit long double */
#ifdef VSTR__LDOUBLE_BITS_96

#define __isinfl vstr__fmt_dbl_isinfl
#ifdef __i386__
# include "glibc/sysdeps/i386/fpu/s_isinfl.c"
#else
# include "glibc/sysdeps/ieee754/ldbl-96/s_isinfl.c"
#endif

#define __isnanl vstr__fmt_dbl_isnanl
#ifdef __i386__
# include "glibc/sysdeps/i386/fpu/s_isnanl.c"
#else
# include "glibc/sysdeps/ieee754/ldbl-96/s_isnanl.c"
#endif

#include "glibc/sysdeps/ieee754/ldbl-96/printf_fphex.c"

#define __mpn_extract_long_double vstr__fmt_dbl_mpn_extract_long_double
#ifdef __i386__
# include "glibc/sysdeps/i386/ldbl2mpn.c"
#else
# include "glibc/sysdeps/ieee754/ldbl-96/ldbl2mpn.c"
#endif

#undef __signbitl
#define __signbitl vstr__fmt_dbl_signbitl
#include "glibc/sysdeps/ieee754/ldbl-96/s_signbitl.c"

#endif

/* Normal double 64 bit support */
#undef  __isinf
#define __isinf vstr__fmt_dbl_isinf
#include "glibc/sysdeps/ieee754/dbl-64/s_isinf.c"

#undef  __isnan
#define __isnan vstr__fmt_dbl_isnan
#include "glibc/sysdeps/ieee754/dbl-64/s_isnan.c"

#undef    signbitf
#undef  __signbitf
#define __signbitf vstr__fmt_dbl_signbitf
#include "glibc/sysdeps/ieee754/flt-32/s_signbitf.c"

#undef    signbit
#undef  __signbit
#define __signbit vstr__fmt_dbl_signbit
#include "glibc/sysdeps/ieee754/dbl-64/s_signbit.c"

/* magic for signbit ... */
#  define signbit(x) \
     (sizeof (x) == sizeof (float)                                            \
      ? __signbitf (x)                                                        \
      : sizeof (x) == sizeof (double)                                         \
      ? __signbit (x) : __signbitl (x))

/* write the number backwards ... */
static char *vstr__fmt_dbl_itoa_word(unsigned long value, char *buflim,
                                     unsigned int base,
                                     int upper_case)
{
  const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";

  if (upper_case)
    digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  switch (base)
  {
    case 16:
      do
        *--buflim = digits[value % 16];
      while (value /= 16);
      break;
    case 10:
      do
        *--buflim = digits[value % 10];
      while (value /= 10);
      break;

    default:
      assert(FALSE);
  }

  return (buflim);
}

static wchar_t *vstr__fmt_dbl_itowa_word(unsigned long value,
                                         wchar_t *buflim,
                                         unsigned int base,
                                         int upper_case)
{
  const wchar_t *digits = L"0123456789abcdefghijklmnopqrstuvwxyz";

  if (upper_case)
    digits = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  switch (base)
  {
    case 16:
      do
        *--buflim = digits[value % 16];
      while (value /= 16);
      break;
    case 10:
      do
        *--buflim = digits[value % 10];
      while (value /= 10);
      break;

    default:
      assert(FALSE);
  }

  return (buflim);
}

#undef _itoa_word
#define _itoa_word(a, b, c, d) vstr__fmt_dbl_itoa_word(a, b, c, d)
#undef _itowa_word
#define _itowa_word(a, b, c, d) vstr__fmt_dbl_itowa_word(a, b, c, d)

static char *vstr__fmt_dbl_itoa(unsigned long long value, char *buflim,
                                unsigned int base, int upper_case)
{
  const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";

  if (upper_case)
    digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  switch (base)
  {
    case 16:
      do
        *--buflim = digits[value % 16];
      while (value /= 16);
      break;
    case 10:
      do
        *--buflim = digits[value % 10];
      while (value /= 10);
      break;

    default:
      assert(FALSE);
  }

  return (buflim);
}

static wchar_t *vstr__fmt_dbl_itowa(unsigned long long value,
                                    wchar_t *buflim,
                                    unsigned int base,
                                    int upper_case)
{
  const wchar_t *digits = L"0123456789abcdefghijklmnopqrstuvwxyz";

  if (upper_case)
    digits = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  switch (base)
  {
    case 16:
      do
        *--buflim = digits[value % 16];
      while (value /= 16);
      break;
    case 10:
      do
        *--buflim = digits[value % 10];
      while (value /= 10);
      break;

    default:
      assert(FALSE);
  }

  return (buflim);
}

#undef  __mpn_extract_double
#define __mpn_extract_double vstr__fmt_dbl_mpn_extract_double
#include "glibc/sysdeps/ieee754/dbl-64/dbl2mpn.c"

#undef __mpn_cmp
#define __mpn_cmp vstr__fmt_dbl_mpn_cmp
#undef mpn_cmp
#define mpn_cmp vstr__fmt_dbl_mpn_cmp
#include "glibc/sysdeps/generic/cmp.c"

extern mp_limb_t
vstr__fmt_dbl_mpn_add_n (mp_ptr res_ptr, mp_srcptr s1_ptr,
                         mp_srcptr s2_ptr, mp_size_t size) VSTR__ATTR_H();
#undef mpn_add_n
#define mpn_add_n vstr__fmt_dbl_mpn_add_n

extern mp_limb_t
vstr__fmt_dbl_mpn_sub_n (mp_ptr res_ptr, mp_srcptr s1_ptr,
                         mp_srcptr s2_ptr, mp_size_t size) VSTR__ATTR_H();
#undef mpn_sub_n
#define mpn_sub_n vstr__fmt_dbl_mpn_sub_n

extern mp_limb_t
vstr__fmt_dbl_mpn_addmul_1(mp_ptr res_ptr, mp_srcptr s1_ptr,
                           mp_size_t s1_size, mp_limb_t s2_limb)
    VSTR__ATTR_H();
#undef mpn_addmul_1
#define mpn_addmul_1 vstr__fmt_dbl_mpn_addmul_1

extern mp_limb_t
vstr__fmt_dbl_mpn_submul_1 (mp_ptr res_ptr, mp_srcptr s1_ptr,
                            mp_size_t s1_size, mp_limb_t s2_limb)
    VSTR__ATTR_H();
#undef mpn_submul_1
#define mpn_submul_1 vstr__fmt_dbl_mpn_submul_1

#undef mpn_divrem
#define mpn_divrem vstr__fmt_dbl_mpn_divrem
#include "glibc/sysdeps/generic/divrem.c"

extern mp_limb_t
vstr__fmt_dbl_mpn_mul_1 (mp_ptr res_ptr, mp_srcptr s1_ptr,
                         mp_size_t s1_size, mp_limb_t s2_limb) VSTR__ATTR_H();
#undef __mpn_mul_1
#define __mpn_mul_1 vstr__fmt_dbl_mpn_mul_1
#undef mpn_mul_1
#define mpn_mul_1 vstr__fmt_dbl_mpn_mul_1

#undef  impn_mul_n_basecase
#define impn_mul_n_basecase vstr__fmt_dbl_impn_mul_n_basecase
#undef  impn_mul_n
#define impn_mul_n vstr__fmt_dbl_impn_mul_n
#include "glibc/sysdeps/generic/mul_n.c"

#undef  __mpn_mul
#define __mpn_mul vstr__fmt_dbl_mpn_mul
#undef  mpn_mul
#define mpn_mul vstr__fmt_dbl_mpn_mul
#include "glibc/sysdeps/generic/mul.c"

extern mp_limb_t
vstr__fmt_dbl_mpn_lshift (register mp_ptr wp,
                          register mp_srcptr up, mp_size_t usize,
                          register unsigned int cnt) VSTR__ATTR_H();
#undef __mpn_lshift
#define __mpn_lshift vstr__fmt_dbl_mpn_lshift

extern mp_limb_t
vstr__fmt_dbl_mpn_rshift (register mp_ptr wp,
                          register mp_srcptr up, mp_size_t usize,
                          register unsigned int cnt) VSTR__ATTR_H();
#undef __mpn_rshift
#define __mpn_rshift vstr__fmt_dbl_mpn_rshift

/* end of MPN code */

#include "glibc/stdlib/fpioconst.h"

#undef  __tens
#define __tens vstr__fmt_dbl_tens
#undef  _fpioconst_pow10
#define _fpioconst_pow10 vstr__fmt_dbl_fpioconst_pow10
#include "glibc/stdlib/fpioconst.c"

/* BEG: printf code ... */
#undef _itoa
#define _itoa(a, b, c, d) vstr__fmt_dbl_itoa(a, b, c, d)
#undef _itowa
#define _itowa(a, b, c, d) vstr__fmt_dbl_itowa(a, b, c, d)

#undef inline


/* Macros for doing the actual output. in both printf functions */

#define outchar(ch)							      \
  do									      \
    {									      \
      register const int outc = (ch);					      \
      if (putc (outc, fp) == EOF)					      \
	return FALSE;							      \
      ++done;								      \
    } while (0)

#define PRINT(ptr, wptr, len)						      \
  do									      \
    {									      \
      register size_t outlen = (len);					      \
      assert(!wide && wptr);						      \
      (void)wptr;							      \
	while (outlen-- > 0)						      \
	  outchar (*ptr++);						      \
    } while (0)

#define PADN(ch, len)							      \
  do									      \
    {									      \
      if (PAD (fp, ch, len) != len)					      \
	return FALSE;							      \
      done += len;							      \
    }									      \
  while (0)

#ifndef MIN
# define MIN(a,b) ((a)<(b)?(a):(b))
#endif


#define __wmemmove(x, y, z) memmove(x, y, (z) * sizeof (wchar_t))
/* internal use function ... we aren't crap like glibc */
#define internal_function /* do nothing */
#define INTUSE(x) x

#define VSTR__FMT_DBL_GLIBC_LOC_DECIMAL_POINT(x) vstr__loc_num_pnt_ptr((x)->base->conf->loc, 10)
#define VSTR__FMT_DBL_GLIBC_LOC_MON_DECIMAL_POINT(x) vstr__loc_num_pnt_ptr((x)->base->conf->loc, 10)
#define VSTR__FMT_DBL_GLIBC_LOC_GROUPING(x) vstr__loc_num_grouping((x)->base->conf->loc, 10)
#define VSTR__FMT_DBL_GLIBC_LOC_MON_GROUPING(x) vstr__loc_num_grouping((x)->base->conf->loc, 10)
#define VSTR__FMT_DBL_GLIBC_LOC_THOUSANDS_SEP(x) vstr__loc_num_sep_ptr((x)->base->conf->loc, 10)
#define VSTR__FMT_DBL_GLIBC_LOC_MON_THOUSANDS_SEP(x) vstr__loc_num_sep_ptr((x)->base->conf->loc, 10)

#define _NL_CURRENT(ignore, db_loc) VSTR__FMT_DBL_GLIBC_LOC_ ## db_loc (fp)

#define _NL_CURRENT_WORD(x, y) L'.'

#undef  __guess_grouping
#define __guess_grouping vstr__fmt_dbl_guess_grouping

#undef  group_number
#define group_number vstr__fmt_dbl_group_number

#undef  __mempcpy
#define __mempcpy mempcpy

#undef  wide
#define wide 0 /* no wide output device, on either fp or fphex */

#undef  extra
#define extra 0 /* no extra output, on either fp or fphex */

#undef  __printf_fp
#define __printf_fp vstr__fmt_printf_fp
#undef  printf_info
#define printf_info vstr__fmt_printf_info
#undef  __long_double_t
#define __long_double_t long double
#undef  exp10
#define exp10 vstr__fmt_printf_exp10
#include "glibc/stdio-common/printf_fp.c"

#ifndef PRINT_FPHEX_LONG_DOUBLE
# error "No long double support for printf_fphex"
#endif

#undef  VSTR__FMT_DBL_GLIBC_LOC_DECIMAL_POINT
#define VSTR__FMT_DBL_GLIBC_LOC_DECIMAL_POINT(x) vstr__loc_num_pnt_ptr((x)->base->conf->loc, 16)
#undef  VSTR__FMT_DBL_GLIBC_LOC_MON_DECIMAL_POINT
#define VSTR__FMT_DBL_GLIBC_LOC_MON_DECIMAL_POINT(x) vstr__loc_num_pnt_ptr((x)->base->conf->loc, 16)

#undef  __printf_fphex
#define __printf_fphex vstr__fmt_printf_fphex
#include "glibc/sysdeps/generic/printf_fphex.c"

static int vstr__add_fmt_dbl(Vstr_base *base, size_t pos_diff,
                             struct Vstr__fmt_spec *spec)
{
  FILE wrap_fp;
  struct vstr__fmt_printf_info info;
  const void *dbl_ptr = NULL;

  wrap_fp.base = base;
  wrap_fp.pos_diff = pos_diff;
  wrap_fp.spec = spec;

  if (spec->flags & PREC_USR)
    info.prec = spec->precision;
  else
    info.prec = -1; /* if they didn't supply them then work it out */

  if (spec->field_width_usr)
    info.width = spec->field_width;
  else
    info.width = 0;

  info.spec = spec->fmt_code;

  info.is_long_double = (spec->int_type == VSTR_TYPE_FMT_ULONG_LONG);
  info.alt =      !!(spec->flags & SPECIAL);
  info.space =    !!(spec->flags & SPACE);
  info.left =     !!(spec->flags & LEFT);
  info.showsign = !!(spec->flags & PLUS);
  info.group =    !!(spec->flags & THOUSAND_SEP);

  if (spec->flags & ZEROPAD)
    info.pad = '0';
  else
    info.pad = ' ';

  if (spec->int_type == VSTR_TYPE_FMT_ULONG_LONG)
    dbl_ptr = &spec->u.data_Ld;
  else
    dbl_ptr = &spec->u.data_d;

  switch (spec->fmt_code)
  {
    case 'a':
    case 'A':
      return (vstr__fmt_printf_fphex(&wrap_fp, &info, &dbl_ptr));
    case 'e':
    case 'E':
    case 'f':
    case 'F':
    case 'g':
    case 'G':
      return (vstr__fmt_printf_fp(&wrap_fp, &info, &dbl_ptr));

    default:
      assert(FALSE);
  }

  return (FALSE);
}

 /* might use these in vstr_add_fmt.c */
#undef wide
#undef extra
#undef  FILE
#undef  isupper
#undef  tolower
#undef _tolower
