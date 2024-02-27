/*
  This file is part of libmicrohttpd
  Copyright (C) 2023 Evgeny Grin (Karlson2k)

  This test tool is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This test tool is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.
  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file microhttpd/test_mhd_version.с
 * @brief  Tests for MHD versions identifiers
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#else  /* ! HAVE_INTTYPES_H */
#define PRIX32 "X"
#endif /* ! HAVE_INTTYPES_H */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#include "microhttpd.h"


#ifdef PACKAGE_VERSION
static const char str_macro_pkg_ver[] = PACKAGE_VERSION;
#else  /* ! PACKAGE_VERSION */
static const char str_macro_pkg_ver[] = "error!";
#error No PACKAGE_VERSION defined
#endif /* ! PACKAGE_VERSION */

#ifdef VERSION
static const char str_macro_ver[] = VERSION;
#else  /* ! VERSION */
static const char str_macro_ver[] = "error!";
#error No PACKAGE_VERSION defined
#endif /* ! VERSION */

#ifdef MHD_VERSION
static const uint32_t bin_macro = (uint32_t) (MHD_VERSION);
#else  /* ! MHD_VERSION */
static const uint32_t bin_macro = 0;
#error MHD_VERSION is not defined
#endif /* ! MHD_VERSION */

/* 0 = success, 1 = failure */
static int
test_macro1_vs_macro2_str (void)
{
  printf ("Checking PACKAGE_VERSION macro vs VERSION macro.\n");
  if (0 != strcmp (str_macro_pkg_ver, str_macro_ver))
  {
    fprintf (stderr, "'%s' vs '%s' - FAILED.\n",
             str_macro_pkg_ver, str_macro_ver);
    return 1;
  }
  printf ("'%s' vs '%s' - success.\n",
          str_macro_pkg_ver, str_macro_ver);
  return 0;
}


/* 0 = success, 1 = failure */
static int
test_macro2_vs_func_str (void)
{
  const char *str_func = MHD_get_version ();
  printf ("Checking VERSION macro vs MHD_get_version() function.\n");
  if (NULL == str_func)
  {
    fprintf (stderr, "MHD_get_version() returned NULL.\n");
    return 1;
  }
  if (0 != strcmp (str_macro_ver, str_func))
  {
    fprintf (stderr, "'%s' vs '%s' - FAILED.\n",
             str_macro_ver, str_func);
    return 1;
  }
  printf ("'%s' vs '%s' - success.\n",
          str_macro_ver, str_func);
  return 0;
}


/* 0 = success, 1 = failure */
static int
test_func_str_vs_macro_bin (void)
{
  char bin_print[64];
  int res;
  const char *str_func = MHD_get_version ();

  printf ("Checking MHD_get_version() function vs MHD_VERSION macro.\n");
#ifdef HAVE_SNPRINTF
  res = snprintf (bin_print, sizeof(bin_print), "%X.%X.%X",
                  (unsigned int) ((bin_macro >> 24) & 0xFF),
                  (unsigned int) ((bin_macro >> 16) & 0xFF),
                  (unsigned int) ((bin_macro >> 8) & 0xFF));
#else  /* ! HAVE_SNPRINTF */
  res = sprintf (bin_print, "%X.%X.%X",
                 (unsigned int) ((bin_macro >> 24) & 0xFF),
                 (unsigned int) ((bin_macro >> 16) & 0xFF),
                 (unsigned int) ((bin_macro >> 8) & 0xFF));
#endif /* ! HAVE_SNPRINTF */
  if ((9 < res) || (0 >= res))
  {
    fprintf (stderr, "snprintf() error.\n");
    exit (99);
  }

  if (0 != strcmp (str_func, bin_print))
  {
    fprintf (stderr, "'%s' vs '0x%08" PRIX32 "' ('%s') - FAILED.\n",
             str_func,
             bin_macro,
             bin_print);
    return 1;
  }
  fprintf (stderr, "'%s' vs '0x%08" PRIX32 "' ('%s') - success.\n",
           str_func,
           bin_macro,
           bin_print);
  return 0;
}


/* 0 = success, 1 = failure */
static int
test_macro_vs_func_bin (void)
{
  const uint32_t bin_func = MHD_get_version_bin ();

  printf ("Checking MHD_VERSION macro vs MHD_get_version_bin() function.\n");
  if (bin_macro != bin_func)
  {
    fprintf (stderr, "'0x%08" PRIX32 "' vs '0x%08" PRIX32 "' - FAILED.\n",
             bin_macro, bin_func);
    return 1;
  }
  printf ("'0x%08" PRIX32 "' vs '0x%08" PRIX32 "' - success.\n",
          bin_macro, bin_func);
  return 0;
}


/* 0 = success, 1 = failure */
static int
test_func_bin_format (void)
{
  const uint32_t bin_func = MHD_get_version_bin ();
  unsigned int test_byte;
  int ret = 0;
  printf ("Checking format of MHD_get_version_bin() function return value.\n");
  test_byte = (unsigned int) ((bin_func >> 24) & 0xFF);
  if ((0xA <= (test_byte & 0xF))
      || (0xA <= (test_byte >> 4)))
  {
    fprintf (stderr,
             "Invalid value in the first (most significant) byte: %02X\n",
             test_byte);
    ret = 1;
  }
  test_byte = (unsigned int) ((bin_func >> 16) & 0xFF);
  if ((0xA <= (test_byte & 0xF))
      || (0xA <= (test_byte >> 4)))
  {
    fprintf (stderr,
             "Invalid value in the second byte: %02X\n",
             test_byte);
    ret = 1;
  }
  test_byte = (unsigned int) ((bin_func >> 8) & 0xFF);
  if ((0xA <= (test_byte & 0xF))
      || (0xA <= (test_byte >> 4)))
  {
    fprintf (stderr,
             "Invalid value in the third byte: %02X\n",
             test_byte);
    ret = 1;
  }
  if (0 != ret)
  {
    fprintf (stderr,
             "The value (0x%08" PRIX32 ") returned by MHD_get_version_bin() "
             "function is invalid as it cannot be used as packed BCD form "
             "(its hexadecimal representation has at least one digit in "
             "A-F range).\n",
             bin_func);
    return 1;
  }
  printf ("'0x%08" PRIX32 "' - success.\n", bin_func);
  return 0;
}


int
main (void)
{
  int res;
  res = test_macro1_vs_macro2_str ();
  res += test_macro2_vs_func_str ();
  res += test_func_str_vs_macro_bin ();
  res += test_macro_vs_func_bin ();
  res += test_func_bin_format ();

  if (0 != res)
  {
    fprintf (stderr, "Test failed. Number of errors: %d\n", res);
    return 1;
  }
  printf ("Test succeed.\n");
  return 0;
}
