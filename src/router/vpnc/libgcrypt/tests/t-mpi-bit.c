/* t-mpi-bit.c  - Tests for bit level functions
 * Copyright (C) 2006 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "../src/gcrypt.h"

#define PGM "t-mpi-bit"

static const char *wherestr;
static int verbose;
static int error_count;

#define xmalloc(a)    gcry_xmalloc ((a))
#define xcalloc(a,b)  gcry_xcalloc ((a),(b))
#define xfree(a)      gcry_free ((a))
#define pass() do { ; } while (0)

static void
show (const char *format, ...)
{
  va_list arg_ptr;

  if (!verbose)
    return;
  fprintf (stderr, "%s: ", PGM);
  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);
}

static void
fail (const char *format, ...)
{
  va_list arg_ptr;

  fflush (stdout);
  fprintf (stderr, "%s: ", PGM);
  if (wherestr)
    fprintf (stderr, "%s: ", wherestr);
  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);
  error_count++;
}

static void
die (const char *format, ...)
{
  va_list arg_ptr;

  fflush (stdout);
  fprintf (stderr, "%s: ", PGM);
  if (wherestr)
    fprintf (stderr, "%s: ", wherestr);
  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);
  exit (1);
}

/* Allocate a bit string consisting of '0' and '1' from the MPI
   A. Return the LENGTH least significant bits. Caller needs to xfree
   the result. */
static char *
mpi2bitstr (gcry_mpi_t a, size_t length)
{
  char *p, *buf;
  
  buf = p = xmalloc (length+1);
  while (length--)
    *p++ = gcry_mpi_test_bit (a, length) ? '1':'0';
  *p = 0;

  return buf;
}

/* Shift a bit string to the right. */
static void
rshiftbitstring (char *string, size_t n)
{
  size_t len = strlen (string);

  if (n > len)
    n = len;

  memmove (string+n, string, len-n);
  memset (string, '0', n);
}


/* This is to check a bug reported by bpgcrypt at itaparica.org on
   2006-07-31 against libgcrypt 1.2.2.  */
static void
one_bit_only (int highbit)
{
  gcry_mpi_t a;
  char *result;
  int i;

  wherestr = "one_bit_only";
  show ("checking that set_%sbit does only set one bit\n", highbit?"high":"");

  a = gcry_mpi_new (0);
  gcry_mpi_randomize (a, 70, GCRY_WEAK_RANDOM);
  gcry_mpi_set_ui (a, 0);

  if (highbit)
    gcry_mpi_set_highbit (a, 42);
  else
    gcry_mpi_set_bit (a, 42);
  if (!gcry_mpi_test_bit (a, 42))
    fail ("failed to set a bit\n");
  gcry_mpi_clear_bit (a, 42);
  if (gcry_mpi_test_bit (a, 42))
    fail ("failed to clear a bit\n");
  result = mpi2bitstr (a, 70);
  assert (strlen (result) == 70);
  show ("r=%s\n", result);
  for (i=0; result[i]; i++)
    if ( result[i] != '0' )
      break;
  if (result[i])
    fail ("spurious bits detected\n");
  xfree (result);
  gcry_mpi_release (a);
}

/* Check that the shifting actually works for an amount larger than
   the number of bits per limb. */
static void
test_rshift (int pass)
{
  gcry_mpi_t a, b;
  char *result, *result2;
  int i;

  wherestr = "test_rshift";
  show ("checking that rshift works as expected (pass %d)\n", pass);

  a = gcry_mpi_new (0);
  b = gcry_mpi_new (0);
  gcry_mpi_randomize (a, 70, GCRY_WEAK_RANDOM);

  for (i=0; i < 75; i++)
    {
      gcry_mpi_rshift (b, a, i);

      result = mpi2bitstr (b, 72);
      result2 = mpi2bitstr (a, 72);
      rshiftbitstring (result2, i);
      if (strcmp (result, result2))
        {
          show ("got =%s\n", result);
          show ("want=%s\n", result2);
          fail ("rshift by %d failed\n", i);
        }
      xfree (result);
      xfree (result2);
    }

  /* Again. This time using in-place operation. */
  gcry_mpi_randomize (a, 70, GCRY_WEAK_RANDOM);

  for (i=0; i < 75; i++)
    {
      gcry_mpi_release (b);
      b = gcry_mpi_copy (a);
      gcry_mpi_rshift (b, b, i);

      result = mpi2bitstr (b, 72);
      result2 = mpi2bitstr (a, 72);
      rshiftbitstring (result2, i);
      if (strcmp (result, result2))
        {
          show ("got =%s\n", result);
          show ("want=%s\n", result2);
          fail ("in-place rshift by %d failed\n", i);
        }
      xfree (result2);
      xfree (result);
    }

  gcry_mpi_release (b);
  gcry_mpi_release (a);
}


int
main (int argc, char **argv)
{
  int debug = 0;
  int i;

  if (argc > 1 && !strcmp (argv[1], "--verbose"))
    verbose = 1;
  else if (argc > 1 && !strcmp (argv[1], "--debug"))
    verbose = debug = 1;

  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");

  gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
  if (debug)
    gcry_control (GCRYCTL_SET_DEBUG_FLAGS, 1u, 0);
  gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);

  one_bit_only (0);
  one_bit_only (1);
  for (i=0; i < 5; i++)
    test_rshift (i); /* Run several times due to random initializations. */
  
  show ("All tests completed. Errors: %d\n", error_count);
  return error_count ? 1 : 0;
}
