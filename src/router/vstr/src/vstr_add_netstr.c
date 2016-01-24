#define VSTR_ADD_NETSTR_C
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
/* netstr (http://cr.yp.to/proto/netstrings.txt). This is basically
 * <num ':' data ','>
 * where
 * num is an ascii number (base 10)
 * data is 8 bit binary data (Ie. any value 0 - 255 is allowed).
 */
/* netstr2 like netstr but allows leading '0' characters */

#include "main.h"

#ifndef VSTR_AUTOCONF_ULONG_MAX_LEN
/* only used as a variable if can't work i tout at compile time */
size_t vstr__netstr2_ULONG_MAX_len = 0;
#endif

size_t vstr_add_netstr2_beg(Vstr_base *base, size_t pos)
{
  size_t tmp = 0;
  size_t ret = 0;

  ASSERT_RET(base, 0);
  ASSERT_RET(pos <= base->len, 0);

  ret = pos + 1;
  /* number will be overwritten so it's ok in OS/compile default locale */
  tmp = vstr_add_sysfmt(base, pos, "%lu%c", ULONG_MAX, VSTR__ASCII_COLON());

  if (!tmp)
    return (0);

  --tmp; /* remove comma from len */

  assert(!VSTR__ULONG_MAX_LEN || (tmp == VSTR__ULONG_MAX_LEN));
  VSTR__ULONG_MAX_SET_LEN(tmp);

  assert(vstr_export_chr(base, ret + VSTR__ULONG_MAX_LEN) ==
         VSTR__ASCII_COLON());

  return (ret);
}

static int vstr__netstr_end_start(Vstr_base *base,
                                  size_t beg_pos, size_t end_pos,
                                  size_t *count, char *buf)
{
  size_t len = 0;

  ASSERT(base);
  ASSERT(beg_pos);
  ASSERT(end_pos);
  
  ASSERT_RET(VSTR__ULONG_MAX_LEN, FALSE);

  ASSERT_RET(beg_pos < end_pos, FALSE);
  ASSERT_RET(end_pos <= base->len, FALSE);
  ASSERT_RET(vstr_sc_posdiff(beg_pos, end_pos) > VSTR__ULONG_MAX_LEN, FALSE);

  assert(vstr_export_chr(base, beg_pos + VSTR__ULONG_MAX_LEN) ==
         VSTR__ASCII_COLON());

  /* includes the ':' */
  len = end_pos - (beg_pos + VSTR__ULONG_MAX_LEN);

  if (!vstr_add_rep_chr(base, end_pos, VSTR__ASCII_COMMA(), 1))
    return (FALSE);

  *count = VSTR__ULONG_MAX_LEN;
  while (len)
  {
    int off = len % 10;

    buf[--*count] = VSTR__ASCII_DIGIT_0() + off;

    len /= 10;
  }

  return (TRUE);
}

int vstr_add_netstr2_end(Vstr_base *base,
                         size_t netstr_beg_pos, size_t netstr_end_pos)
{
  size_t count = 0;
  char buf[BUF_NUM_TYPE_SZ(unsigned long)];

  assert(sizeof(buf) >= VSTR__ULONG_MAX_LEN);

  if (!vstr__netstr_end_start(base, netstr_beg_pos, netstr_end_pos,
                              &count, buf))
    return (FALSE);
  assert(count <= VSTR__ULONG_MAX_LEN);

  /* must use sub_buf ... as sub_rep_buf isn't as optimised ...
   * so it'd be slower _and_ we'd need to check/handle for failure */
  vstr_wrap_memset(buf, VSTR__ASCII_DIGIT_0(), count);
  vstr_sub_buf(base, netstr_beg_pos, count, buf, count);

  vstr_sub_buf(base, netstr_beg_pos + count, VSTR__ULONG_MAX_LEN - count,
               buf + count, VSTR__ULONG_MAX_LEN - count);

  return (TRUE);
}

/* NOTE: might want to use vstr_add_pos_buf()/_ref() eventually */
size_t vstr_add_netstr_beg(Vstr_base *base, size_t pos)
{
  return (vstr_add_netstr2_beg(base, pos));
}

int vstr_add_netstr_end(Vstr_base *base,
                        size_t netstr_beg_pos, size_t netstr_end_pos)
{
  size_t count = 0;
  char buf[BUF_NUM_TYPE_SZ(unsigned long)];

  assert(sizeof(buf) >= VSTR__ULONG_MAX_LEN);

  if (!vstr__netstr_end_start(base, netstr_beg_pos, netstr_end_pos,
                              &count, buf))
    return (FALSE);
  ASSERT(count <= VSTR__ULONG_MAX_LEN);

  if (count == VSTR__ULONG_MAX_LEN)
  { /* here we delete, so need to keep something */
    buf[--count] = VSTR__ASCII_DIGIT_0();
  }

  if (count && !vstr_del(base, netstr_beg_pos, count))
  {
    vstr_del(base, netstr_end_pos + 1, 1); /* remove comma, always works */
    return (FALSE);
  }
  vstr_sub_buf(base, netstr_beg_pos, VSTR__ULONG_MAX_LEN - count,
               buf + count, VSTR__ULONG_MAX_LEN - count);

  return (TRUE);
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(add_netstr2_beg);
VSTR__SYM_ALIAS(add_netstr2_end);
VSTR__SYM_ALIAS(add_netstr_beg);
VSTR__SYM_ALIAS(add_netstr_end);
