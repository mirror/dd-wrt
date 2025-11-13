/* Test of <stddef.h> substitute.
   Copyright (C) 2009-2022 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Eric Blake <ebb9@byu.net>, 2009.  */

#include <config.h>

#include <stddef.h>
#include <limits.h>
#include <stdalign.h>

/* Check that appropriate types are defined.  */
wchar_t a = 'c';
ptrdiff_t b = 1;
size_t c = 2;
max_align_t x;

/* Check that NULL can be passed through varargs as a pointer type,
   per POSIX 2008.  */
static_assert (sizeof NULL == sizeof (void *));

/* Check that offsetof produces integer constants with correct type.  */
struct d
{
  char e;
  char f;
};
/* Solaris 10 has a bug where offsetof is under-parenthesized, and
   cannot be used as an arbitrary expression.  However, since it is
   unlikely to bite real code, we ignore that short-coming.  */
/* static_assert (sizeof offsetof (struct d, e) == sizeof (size_t)); */
static_assert (sizeof (offsetof (struct d, e)) == sizeof (size_t));
static_assert (offsetof (struct d, f) == 1);

/* offsetof promotes to an unsigned integer if and only if sizes do
   not fit in int.  */
static_assert ((offsetof (struct d, e) < -1) == (INT_MAX < (size_t) -1));

/* Check max_align_t's alignment.  */
static_assert (alignof (double) <= alignof (max_align_t));
static_assert (alignof (int) <= alignof (max_align_t));
static_assert (alignof (long double) <= alignof (max_align_t));
static_assert (alignof (long int) <= alignof (max_align_t));
static_assert (alignof (ptrdiff_t) <= alignof (max_align_t));
static_assert (alignof (size_t) <= alignof (max_align_t));
static_assert (alignof (wchar_t) <= alignof (max_align_t));
static_assert (alignof (struct d) <= alignof (max_align_t));
#if defined __GNUC__ || defined __clang__ || defined __IBM__ALIGNOF__
static_assert (__alignof__ (double) <= __alignof__ (max_align_t));
static_assert (__alignof__ (int) <= __alignof__ (max_align_t));
static_assert (__alignof__ (long double) <= __alignof__ (max_align_t));
static_assert (__alignof__ (long int) <= __alignof__ (max_align_t));
static_assert (__alignof__ (ptrdiff_t) <= __alignof__ (max_align_t));
static_assert (__alignof__ (size_t) <= __alignof__ (max_align_t));
static_assert (__alignof__ (wchar_t) <= __alignof__ (max_align_t));
static_assert (__alignof__ (struct d) <= __alignof__ (max_align_t));
#endif

int
main (void)
{
  return 0;
}
