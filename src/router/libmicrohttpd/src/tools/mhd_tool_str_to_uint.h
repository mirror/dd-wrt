/*
  This file is part of GNU libmicrohttpd
  Copyright (C) 2023 Evgeny Grin (Karlson2k)

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
 * @file tools/mhd_tool_str_to_uint.h
 * @brief  Function to decode the value of decimal string number.
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_TOOL_STR_TO_UINT_H_
#define MHD_TOOL_STR_TO_UINT_H_ 1

#include <stddef.h>

/**
 * Convert decimal string to unsigned int.
 * Function stops at the end of the string or on first non-digit character.
 * @param str the string to convert
 * @param[out] value the pointer to put the result
 * @return return the number of digits converted or
 *         zero if no digits found or result would overflow the output
 *         variable (the output set to UINT_MAX in this case).
 */
static size_t
mhd_tool_str_to_uint (const char *str, unsigned int *value)
{
  size_t i;
  unsigned int v = 0;
  *value = 0;

  for (i = 0; 0 != str[i]; ++i)
  {
    const char chr = str[i];
    unsigned int digit;
    if (('0' > chr) || ('9' < chr))
      break;
    digit = (unsigned char) (chr - '0');
    if ((((0U - 1) / 10) < v) || ((v * 10 + digit) < v))
    {
      /* Overflow */
      *value = 0U - 1;
      return 0;
    }
    v *= 10;
    v += digit;
  }
  *value = v;
  return i;
}


#endif /* MHD_TOOL_STR_TO_UINT_H_ */
