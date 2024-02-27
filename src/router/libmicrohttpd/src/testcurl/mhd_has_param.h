/*
  This file is part of libmicrohttpd
  Copyright (C) 2016-2022 Karlson2k (Evgeny Grin)

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
 * @file testcurl/mhd_has_param.h
 * @brief Static functions and macros helpers for testsuite.
 * @author Karlson2k (Evgeny Grin)
 */

#include <string.h>


/**
 * Check whether one of strings in array is equal to @a param.
 * String @a argv[0] is ignored.
 * @param argc number of strings in @a argv, as passed to main function
 * @param argv array of strings, as passed to main function
 * @param param parameter to look for.
 * @return zero if @a argv is NULL, @a param is NULL or empty string,
 *         @a argc is less then 2 or @a param is not found in @a argv,
 *         non-zero if one of strings in @a argv is equal to @a param.
 */
static int
has_param (int argc, char *const argv[], const char *param)
{
  int i;
  if (! argv || ! param || ! param[0])
    return 0;

  for (i = 1; i < argc; i++)
  {
    if (argv[i] && (strcmp (argv[i], param) == 0) )
      return ! 0;
  }

  return 0;
}
