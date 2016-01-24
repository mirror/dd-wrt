#define VSTR_PARSE_NETSTR_C
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
/* netstr2 like netstr (http://cr.yp.to/proto/netstrings.txt)
 * but allows leading '0' characters */

#include "main.h"

static size_t vstr__parse_netstr(const Vstr_base *base, size_t pos, size_t len,
                                 size_t *ret_pos, size_t *ret_data_len,
                                 int netstr2)
{
  unsigned int flags = VSTR_FLAG_PARSE_NUM_OVERFLOW;
  size_t num_len = 0;
  size_t ret_len = 0;
  size_t dummy_ret_pos = 0;
  size_t dummy_ret_data_len = 0;

  if (!ret_pos)
    ret_pos = &dummy_ret_pos;
  if (!ret_data_len)
    ret_data_len = &dummy_ret_data_len;

  *ret_pos = 0;
  *ret_data_len = 0;

  if (!netstr2)
    flags |= VSTR_FLAG_PARSE_NUM_NO_BEG_ZERO;
  ret_len = vstr_parse_ulong(base, pos, len, 10 | flags, &num_len, NULL);

  if (!num_len)
    return (0);

  if (num_len == len)
    return (0);

  if (vstr_export_chr(base, pos + num_len) != VSTR__ASCII_COLON())
    return (0);

  *ret_pos = pos + num_len + 1;
  *ret_data_len = ret_len;

  ret_len += (num_len + 2); /* colon and comma */
  if (ret_len > len)
    return (0);

  if (vstr_export_chr(base, pos - 1 + ret_len) != VSTR__ASCII_COMMA())
    return (0);

  return (ret_len);
}

size_t vstr_parse_netstr2(const Vstr_base *base, size_t pos, size_t len,
                          size_t *ret_pos, size_t *ret_len)
{
  return (vstr__parse_netstr(base, pos, len, ret_pos, ret_len, TRUE));
}

size_t vstr_parse_netstr(const Vstr_base *base, size_t pos, size_t len,
                         size_t *ret_pos, size_t *ret_len)
{
  return (vstr__parse_netstr(base, pos, len, ret_pos, ret_len, FALSE));
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(parse_netstr);
VSTR__SYM_ALIAS(parse_netstr2);
