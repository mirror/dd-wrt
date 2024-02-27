/*
  This file is part of libmicrohttpd
  Copyright (C) 2015-2022 Karlson2k (Evgeny Grin)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
 * @file microhttpd/mhd_str_types.h
 * @brief  Header for string manipulating helpers types
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_STR_TYPES_H
#define MHD_STR_TYPES_H 1

#ifndef MHD_STATICSTR_LEN_
/**
 * Determine length of static string / macro strings at compile time.
 */
#define MHD_STATICSTR_LEN_(macro) (sizeof(macro) / sizeof(char) - 1)
#endif /* ! MHD_STATICSTR_LEN_ */

/**
 * Constant string with length
 */
struct _MHD_cstr_w_len
{
  const char *const str;
  const size_t len;
};

/**
 * String with length
 */
struct _MHD_str_w_len
{
  const char *str;
  size_t len;
};

/**
 * Modifiable string with length
 */
struct _MHD_mstr_w_len
{
  char *str;
  size_t len;
};

/**
 * Static string initialiser for struct _MHD_str_w_len
 */
#define _MHD_S_STR_W_LEN(str) { str, MHD_STATICSTR_LEN_(str) }

#endif /* MHD_STR_TYPES_H */
